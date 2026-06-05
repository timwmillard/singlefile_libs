package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"github.com/rqlite/sql"
	"gopkg.in/yaml.v3"
)

type NamingStyle string

const (
	SnakeCase  NamingStyle = "snake"
	CamelCase  NamingStyle = "camel"
	PascalCase NamingStyle = "pascal"
)

// Config holds all configuration options
type Config struct {
	Schema      string `yaml:"schema"`
	Queries     string `yaml:"queries"`
	Output      string `yaml:"output"`
	StructStyle string `yaml:"struct-style"`
	FieldStyle  string `yaml:"field-style"`
	FuncStyle   string `yaml:"func-style"`
	TypePrefix  string `yaml:"type-prefix"`
	FuncPrefix  string `yaml:"func-prefix"`
}

// Column represents a table column
type Column struct {
	Name     string
	Type     string
	Nullable bool
	IsPK     bool
}

// Table represents a database table
type Table struct {
	Name    string
	Columns []Column
}

// Query represents a parsed query
type Query struct {
	Name       string
	ReturnType string // "one", "many", "exec"
	SQL        string
	Table      string   // primary table being queried
	Params     []Param  // query parameters
	Columns    []Column // columns being selected (for SELECT queries)
}

// Param represents a query parameter
type Param struct {
	Name string
	Type string // sql_int64, sql_text, etc.
}

type Generator struct {
	tables      map[string]*Table
	queries     []Query
	out         strings.Builder
	implOut     strings.Builder
	modelsOut   strings.Builder
	structStyle NamingStyle
	fieldStyle  NamingStyle
	funcStyle   NamingStyle
	typePrefix  string
	funcPrefix  string
}

func loadConfig(path string) (*Config, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, err
	}
	var cfg Config
	if err := yaml.Unmarshal(data, &cfg); err != nil {
		return nil, err
	}
	return &cfg, nil
}

func main() {
	// Default config
	cfg := Config{
		Schema:      "schema.sql",
		Queries:     "queries.sql",
		Output:      "queries.h",
		StructStyle: "pascal",
		FieldStyle:  "pascal",
		FuncStyle:   "snake",
	}

	// First pass: just look for -config flag
	configPath := ""
	for i, arg := range os.Args[1:] {
		if arg == "-config" && i+1 < len(os.Args)-1 {
			configPath = os.Args[i+2]
			break
		}
		if strings.HasPrefix(arg, "-config=") {
			configPath = strings.TrimPrefix(arg, "-config=")
			break
		}
	}

	// Load config file if specified
	if configPath != "" {
		fileCfg, err := loadConfig(configPath)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error loading config: %v\n", err)
			os.Exit(1)
		}
		if fileCfg.Schema != "" {
			cfg.Schema = fileCfg.Schema
		}
		if fileCfg.Queries != "" {
			cfg.Queries = fileCfg.Queries
		}
		if fileCfg.Output != "" {
			cfg.Output = fileCfg.Output
		}
		if fileCfg.StructStyle != "" {
			cfg.StructStyle = fileCfg.StructStyle
		}
		if fileCfg.FieldStyle != "" {
			cfg.FieldStyle = fileCfg.FieldStyle
		}
		if fileCfg.FuncStyle != "" {
			cfg.FuncStyle = fileCfg.FuncStyle
		}
		if fileCfg.TypePrefix != "" {
			cfg.TypePrefix = fileCfg.TypePrefix
		}
		if fileCfg.FuncPrefix != "" {
			cfg.FuncPrefix = fileCfg.FuncPrefix
		}
	}

	// Define flags with config values as defaults
	configFlag := flag.String("config", "", "Path to config file (YAML)")
	schemaPath := flag.String("schema", cfg.Schema, "Path to schema.sql file")
	queriesPath := flag.String("queries", cfg.Queries, "Path to queries.sql file")
	output := flag.String("output", cfg.Output, "Output header file")
	structStyle := flag.String("struct-style", cfg.StructStyle, "Naming style for structs: snake, camel, pascal")
	fieldStyle := flag.String("field-style", cfg.FieldStyle, "Naming style for fields: snake, camel, pascal")
	funcStyle := flag.String("func-style", cfg.FuncStyle, "Naming style for functions: snake, camel, pascal")
	typePrefix := flag.String("type-prefix", cfg.TypePrefix, "Prefix for type names")
	funcPrefix := flag.String("func-prefix", cfg.FuncPrefix, "Prefix for function names")
	flag.Parse()

	_ = configFlag

	// Read input files
	schemaSQL, err := os.ReadFile(*schemaPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error reading schema: %v\n", err)
		os.Exit(1)
	}

	queriesSQL, err := os.ReadFile(*queriesPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error reading queries: %v\n", err)
		os.Exit(1)
	}

	gen := &Generator{
		tables:      make(map[string]*Table),
		structStyle: NamingStyle(*structStyle),
		fieldStyle:  NamingStyle(*fieldStyle),
		funcStyle:   NamingStyle(*funcStyle),
		typePrefix:  *typePrefix,
		funcPrefix:  *funcPrefix,
	}

	// Parse schema
	if err := gen.parseSchema(string(schemaSQL)); err != nil {
		fmt.Fprintf(os.Stderr, "Error parsing schema: %v\n", err)
		os.Exit(1)
	}

	// Parse queries
	if err := gen.parseQueries(string(queriesSQL)); err != nil {
		fmt.Fprintf(os.Stderr, "Error parsing queries: %v\n", err)
		os.Exit(1)
	}

	// Generate output
	gen.generate(*output)

	// Create output directory if needed
	if dir := filepath.Dir(*output); dir != "." && dir != "" {
		if err := os.MkdirAll(dir, 0755); err != nil {
			fmt.Fprintf(os.Stderr, "Error creating directory: %v\n", err)
			os.Exit(1)
		}
	}

	// Write header file
	if err := os.WriteFile(*output, []byte(gen.out.String()), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "Error writing output: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("Generated %s\n", *output)

	// Write implementation file
	implFile := strings.TrimSuffix(*output, filepath.Ext(*output)) + ".c"
	if err := os.WriteFile(implFile, []byte(gen.implOut.String()), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "Error writing implementation: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("Generated %s\n", implFile)

	// Write models file
	modelsFile := filepath.Join(filepath.Dir(*output), "models.h")
	if err := os.WriteFile(modelsFile, []byte(gen.modelsOut.String()), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "Error writing models: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("Generated %s\n", modelsFile)
}

// parseSchema parses CREATE TABLE statements from schema.sql using the SQL parser
func (g *Generator) parseSchema(sqlText string) error {
	// Strip triggers and DML that the parser can't handle (e.g. rowid references).
	// Only CREATE TABLE/VIRTUAL TABLE statements are needed.
	reTrigger := regexp.MustCompile(`(?is)create\s+trigger\b.*?\bend\s*;`)
	sqlText = reTrigger.ReplaceAllString(sqlText, "")
	reDML := regexp.MustCompile(`(?im)^\s*(delete|insert|drop)\b[^;]*;`)
	sqlText = reDML.ReplaceAllString(sqlText, "")

	parser := sql.NewParser(strings.NewReader(sqlText))
	stmts, err := parser.ParseStatements()
	if err != nil {
		return fmt.Errorf("parsing SQL: %w", err)
	}

	for _, stmt := range stmts {
		createStmt, ok := stmt.(*sql.CreateTableStatement)
		if !ok {
			continue
		}

		tableName := createStmt.Name.Name
		table := &Table{Name: tableName}

		for _, colDef := range createStmt.Columns {
			col := Column{
				Name:     colDef.Name.Name,
				Nullable: true, // Default to nullable
			}

			// Get type name
			if colDef.Type != nil {
				col.Type = strings.ToLower(colDef.Type.Name.Name)
			}

			// Check constraints
			for _, constraint := range colDef.Constraints {
				switch c := constraint.(type) {
				case *sql.PrimaryKeyConstraint:
					col.IsPK = true
					col.Nullable = false
				case *sql.NotNullConstraint:
					col.Nullable = false
				case *sql.DefaultConstraint:
					_ = c // Has default, keep nullable status
				}
			}

			table.Columns = append(table.Columns, col)
		}

		g.tables[tableName] = table
	}

	return nil
}

// parseQueries parses query definitions from queries.sql
func (g *Generator) parseQueries(sql string) error {
	// Split by "-- name:" and parse each block
	blocks := regexp.MustCompile(`(?m)^--\s*name:`).Split(sql, -1)

	for _, block := range blocks[1:] { // Skip first empty block
		block = strings.TrimSpace(block)
		if block == "" {
			continue
		}

		// First line has: QueryName :type
		lines := strings.SplitN(block, "\n", 2)
		if len(lines) < 2 {
			continue
		}

		headerRe := regexp.MustCompile(`(\w+)\s+:(\w+)`)
		headerMatch := headerRe.FindStringSubmatch(lines[0])
		if headerMatch == nil {
			continue
		}

		query := Query{
			Name:       headerMatch[1],
			ReturnType: headerMatch[2],
			SQL:        strings.TrimSpace(lines[1]),
		}

		// Parse the SQL to extract table, params, and columns
		g.parseQuerySQL(&query)
		g.queries = append(g.queries, query)
	}

	return nil
}

// parseQuerySQL analyzes a SQL statement to extract metadata using the SQL parser
func (g *Generator) parseQuerySQL(q *Query) {
	// Quote identifiers in dotted references (e.g., table.rowid -> table."rowid")
	// to work around the parser treating some valid SQLite identifiers as keywords.
	preprocessed := regexp.MustCompile(`(\w)\.(\w+)`).ReplaceAllString(q.SQL, `$1."$2"`)
	parser := sql.NewParser(strings.NewReader(preprocessed))
	stmt, err := parser.ParseStatement()
	if err != nil {
		// Fall back to simple detection if parsing fails
		return
	}

	switch s := stmt.(type) {
	case *sql.SelectStatement:
		g.parseSelectStatement(q, s)
	case *sql.InsertStatement:
		g.parseInsertStatement(q, s)
	case *sql.UpdateStatement:
		g.parseUpdateStatement(q, s)
	case *sql.DeleteStatement:
		g.parseDeleteStatement(q, s)
	}
}

// parseSelectStatement parses a SELECT statement using the AST
func (g *Generator) parseSelectStatement(q *Query, s *sql.SelectStatement) {
	// Extract table name and alias from source (handles JOINs by taking the left-most table)
	var tableAlias string
	if s.Source != nil {
		q.Table, tableAlias = g.extractPrimaryTable(s.Source)
	}

	// Check if SELECT * or SELECT alias.* (all columns)
	allColumns := false
	if len(s.Columns) == 1 {
		col := s.Columns[0]
		if col.Star.IsValid() {
			allColumns = true
		} else if ref, ok := col.Expr.(*sql.QualifiedRef); ok && ref.Star.IsValid() {
			if ref.Table.Name == q.Table || ref.Table.Name == tableAlias {
				allColumns = true
			}
		}
	}
	if allColumns {
		if table, ok := g.tables[q.Table]; ok {
			q.Columns = table.Columns
		}
	} else {
		// Handle individual column selections
		table := g.tables[q.Table]
		if table != nil {
			for _, col := range s.Columns {
				// Get column name from expression
				var colName string
				if col.Expr != nil {
					if ident, ok := col.Expr.(*sql.Ident); ok {
						colName = ident.Name
					}
				}
				if colName == "" {
					continue
				}
				// Look up column type from table definition
				for _, tableCol := range table.Columns {
					if strings.EqualFold(tableCol.Name, colName) {
						q.Columns = append(q.Columns, tableCol)
						break
					}
				}
			}
		}
	}

	// Extract WHERE clause parameters
	if s.WhereExpr != nil {
		g.extractExprParams(q, s.WhereExpr)
	}
}

// extractPrimaryTable recursively finds the primary (left-most) table name and alias
// from a source, handling JOINs by traversing into the left side.
func (g *Generator) extractPrimaryTable(source sql.Source) (name, alias string) {
	switch src := source.(type) {
	case *sql.QualifiedTableName:
		name = src.Name.Name
		if src.Alias != nil {
			alias = src.Alias.Name
		}
	case *sql.JoinClause:
		return g.extractPrimaryTable(src.X)
	}
	return
}

// parseInsertStatement parses an INSERT statement using the AST
func (g *Generator) parseInsertStatement(q *Query, s *sql.InsertStatement) {
	// Extract table name
	if s.Table != nil {
		q.Table = s.Table.Name
	}

	table := g.tables[q.Table]

	// Extract column names and match to params
	for _, col := range s.Columns {
		colName := col.Name
		if table != nil {
			for _, tableCol := range table.Columns {
				if strings.EqualFold(tableCol.Name, colName) {
					q.Params = append(q.Params, Param{
						Name: tableCol.Name,
						Type: g.sqliteTypeToSQLType(tableCol.Type, false),
					})
					break
				}
			}
		}
	}

	// Check for RETURNING clause
	if s.ReturningClause != nil {
		g.extractReturningColumns(q, s.ReturningClause.Columns)
	}
}

// parseUpdateStatement parses an UPDATE statement using the AST
func (g *Generator) parseUpdateStatement(q *Query, s *sql.UpdateStatement) {
	// Extract table name
	if s.Table != nil && s.Table.Name != nil {
		q.Table = s.Table.Name.Name
	}

	table := g.tables[q.Table]

	// Extract SET clause parameters
	for _, assign := range s.Assignments {
		// Check if RHS is a bind parameter (?)
		if _, ok := assign.Expr.(*sql.BindExpr); ok {
			for _, col := range assign.Columns {
				colName := col.Name
				if table != nil {
					for _, tableCol := range table.Columns {
						if strings.EqualFold(tableCol.Name, colName) {
							q.Params = append(q.Params, Param{
								Name: tableCol.Name,
								Type: g.sqliteTypeToSQLType(tableCol.Type, false),
							})
							break
						}
					}
				}
			}
		}
	}

	// Extract WHERE clause parameters
	if s.WhereExpr != nil {
		g.extractExprParams(q, s.WhereExpr)
	}

	// Check for RETURNING clause
	if s.ReturningClause != nil {
		g.extractReturningColumns(q, s.ReturningClause.Columns)
	}
}

// parseDeleteStatement parses a DELETE statement using the AST
func (g *Generator) parseDeleteStatement(q *Query, s *sql.DeleteStatement) {
	// Extract table name
	if s.Table != nil && s.Table.Name != nil {
		q.Table = s.Table.Name.Name
	}

	// Extract WHERE clause parameters
	if s.WhereExpr != nil {
		g.extractExprParams(q, s.WhereExpr)
	}
}

// extractExprParams extracts bind parameters from an expression
func (g *Generator) extractExprParams(q *Query, expr sql.Expr) {
	table := g.tables[q.Table]
	if table == nil {
		return
	}

	// Walk the expression tree to find column = ? and table MATCH ? patterns
	g.walkExpr(expr, func(e sql.Expr) {
		binExpr, ok := e.(*sql.BinaryExpr)
		if !ok {
			return
		}

		switch binExpr.Op {
		case sql.EQ:
			// Check for column = ?
			var colName string
			if ident, ok := binExpr.X.(*sql.Ident); ok {
				if _, ok := binExpr.Y.(*sql.BindExpr); ok {
					colName = ident.Name
				}
			}
			// Also check ? = column (reversed)
			if ident, ok := binExpr.Y.(*sql.Ident); ok {
				if _, ok := binExpr.X.(*sql.BindExpr); ok {
					colName = ident.Name
				}
			}

			if colName != "" {
				for _, col := range table.Columns {
					if strings.EqualFold(col.Name, colName) {
						q.Params = append(q.Params, Param{
							Name: col.Name,
							Type: g.sqliteTypeToSQLType(col.Type, false),
						})
						break
					}
				}
			}

		case sql.MATCH:
			// Handle FTS MATCH ? (e.g., competitor_fts MATCH ?)
			if _, ok := binExpr.Y.(*sql.BindExpr); ok {
				paramName := "query"
				if ident, ok := binExpr.X.(*sql.Ident); ok {
					paramName = ident.Name
				}
				q.Params = append(q.Params, Param{
					Name: paramName,
					Type: "sql_text",
				})
			}
		}
	})
}

// extractReturningColumns extracts columns from a RETURNING clause
func (g *Generator) extractReturningColumns(q *Query, columns []*sql.ResultColumn) {
	table := g.tables[q.Table]
	if table == nil {
		return
	}

	for _, rc := range columns {
		// Check if it's a star (RETURNING *)
		if rc.Star.IsValid() {
			q.Columns = table.Columns
			return
		}
		// Handle individual column
		if rc.Expr != nil {
			if ident, ok := rc.Expr.(*sql.Ident); ok {
				for _, tableCol := range table.Columns {
					if strings.EqualFold(tableCol.Name, ident.Name) {
						q.Columns = append(q.Columns, tableCol)
						break
					}
				}
			}
		}
	}
}

// walkExpr walks an expression tree calling fn for each node
func (g *Generator) walkExpr(expr sql.Expr, fn func(sql.Expr)) {
	if expr == nil {
		return
	}

	fn(expr)

	switch e := expr.(type) {
	case *sql.BinaryExpr:
		g.walkExpr(e.X, fn)
		g.walkExpr(e.Y, fn)
	case *sql.UnaryExpr:
		g.walkExpr(e.X, fn)
	case *sql.ParenExpr:
		g.walkExpr(e.X, fn)
	case *sql.CaseExpr:
		g.walkExpr(e.Operand, fn)
		for _, blk := range e.Blocks {
			g.walkExpr(blk.Condition, fn)
			g.walkExpr(blk.Body, fn)
		}
		g.walkExpr(e.ElseExpr, fn)
	}
}

// sqliteTypeToSQLType converts SQLite type to sql_* type
func (g *Generator) sqliteTypeToSQLType(sqliteType string, nullable bool) string {
	sqliteType = strings.ToLower(sqliteType)

	var baseType string
	switch {
	case strings.Contains(sqliteType, "int"):
		baseType = "sql_int64"
	case strings.Contains(sqliteType, "text"), strings.Contains(sqliteType, "char"):
		baseType = "sql_text"
	case strings.Contains(sqliteType, "real"), strings.Contains(sqliteType, "double"), strings.Contains(sqliteType, "float"):
		baseType = "sql_double"
	case strings.Contains(sqliteType, "blob"):
		baseType = "sql_blob"
	case strings.Contains(sqliteType, "bool"):
		baseType = "sql_bool"
	case strings.Contains(sqliteType, "numeric"):
		baseType = "sql_numeric"
	default:
		baseType = "sql_text" // Default to text
	}

	if nullable {
		return "sql_null" + strings.TrimPrefix(baseType, "sql_")
	}
	return baseType
}

// generate produces the C header and implementation files
func (g *Generator) generate(outputFile string) {
	g.generateModels()
	g.generateHeader(outputFile)
	g.generateImplementation(outputFile)
}

// generateModels produces the models.h file with base types and table structs
func (g *Generator) generateModels() {
	g.modelsOut.WriteString("// Generated from SQL - do not edit\n\n")
	g.modelsOut.WriteString("#ifndef SQL_MODEL_H\n")
	g.modelsOut.WriteString("#define SQL_MODEL_H\n\n")
	g.modelsOut.WriteString("#include <stdint.h>\n")
	g.modelsOut.WriteString("#include <stdbool.h>\n")
	g.modelsOut.WriteString("#include <stddef.h>\n")
	g.modelsOut.WriteString("#include <string.h>\n\n")

	// Hardcoded base types
	g.modelsOut.WriteString(`typedef unsigned char sql_byte;

typedef double sql_double;

typedef struct {
    sql_double value;
    bool null;
} sql_nulldouble;

typedef int sql_int;

typedef struct {
    sql_int value;
    bool null;
} sql_nullint;

typedef int64_t sql_int64;

typedef struct {
    sql_int64 value;
    bool null;
} sql_nullint64;

typedef double sql_numeric;

typedef struct {
    sql_numeric value;
    bool null;
} sql_nullnumeric;

typedef bool sql_bool;

typedef struct {
    sql_bool value;
    bool null;
} sql_nullbool;

typedef struct {
    sql_byte *data;
    size_t len;
} sql_blob;

typedef struct {
    sql_byte *data;
    size_t len;
    bool null;
} sql_nullblob;

typedef struct {
    sql_byte *data;
    size_t len;
} sql_text;

static inline sql_text to_sql_text(char *text) {
    return (sql_text){
        .data = (sql_byte*)text,
        .len = strlen(text),
    };
}

typedef struct {
    char *data;
    size_t len;
    bool null;
} sql_nulltext;

static inline sql_nulltext to_sql_nulltext(char *text, bool null) {
    return (sql_nulltext){
        .data = text,
        .len = strlen(text),
        .null = null,
    };
}

`)

	// Generate table structs
	g.modelsOut.WriteString("// ============ Table Structs ============\n\n")
	for _, tableName := range g.sortedTableNames() {
		table := g.tables[tableName]
		structName := g.typeName(table.Name)

		g.modelsOut.WriteString("typedef struct {\n")
		for _, col := range table.Columns {
			fieldName := g.applyStyle(col.Name, g.fieldStyle)
			fieldType := g.sqliteTypeToSQLType(col.Type, col.Nullable)
			g.modelsOut.WriteString(fmt.Sprintf("    %s %s;\n", fieldType, fieldName))
		}
		g.modelsOut.WriteString(fmt.Sprintf("} %s;\n\n", structName))
	}

	g.modelsOut.WriteString("#endif // SQL_MODEL_H\n")
}

// generateHeader produces the .h file
func (g *Generator) generateHeader(outputFile string) {
	guardName := strings.ToUpper(strings.ReplaceAll(filepath.Base(outputFile), ".", "_"))

	g.out.WriteString("// Generated from SQL - do not edit\n\n")
	g.out.WriteString(fmt.Sprintf("#ifndef %s\n", guardName))
	g.out.WriteString(fmt.Sprintf("#define %s\n\n", guardName))
	g.out.WriteString("#include \"sqlite3.h\"\n\n")
	g.out.WriteString("#include \"models.h\"\n\n")

	// Generate param structs for queries with multiple params
	g.out.WriteString("// ============ Param Structs ============\n\n")
	for _, q := range g.queries {
		if len(q.Params) > 1 {
			g.generateParamStruct(q)
		}
	}

	// Generate function declarations
	g.out.WriteString("// ============ Query Functions ============\n\n")
	for _, q := range g.queries {
		g.generateFunctionDecl(q)
	}

	g.out.WriteString("\n")
	g.out.WriteString(fmt.Sprintf("#endif // %s\n", guardName))
}

// generateParamStruct generates a params struct for a query
func (g *Generator) generateParamStruct(q Query) {
	structName := g.typeName(q.Name + "Params")

	g.out.WriteString("typedef struct {\n")
	for _, p := range q.Params {
		fieldName := g.applyStyle(p.Name, g.fieldStyle)
		g.out.WriteString(fmt.Sprintf("    %s %s;\n", p.Type, fieldName))
	}
	g.out.WriteString(fmt.Sprintf("} %s;\n\n", structName))
}

// generateFunctionDecl generates a function declaration
func (g *Generator) generateFunctionDecl(q Query) {
	funcName := g.funcName(q.Name)
	resultType := g.typeName(q.Table)

	// Format SQL as single-line comment
	sqlComment := strings.ReplaceAll(q.SQL, "\n", " ")
	sqlComment = strings.Join(strings.Fields(sqlComment), " ") // normalize whitespace
	g.out.WriteString(fmt.Sprintf("// %s\n", sqlComment))

	switch q.ReturnType {
	case "one":
		if len(q.Params) == 0 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, void (*cb)(%s*, void*), void *ctx);\n", funcName, resultType))
		} else if len(q.Params) == 1 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s, void (*cb)(%s*, void*), void *ctx);\n",
				funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle), resultType))
		} else {
			paramsType := g.typeName(q.Name + "Params")
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params, void (*cb)(%s*, void*), void *ctx);\n",
				funcName, paramsType, resultType))
		}

	case "many":
		// Function signature
		if len(q.Params) == 0 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, void (*cb)(%s*, void*), void *ctx);\n", funcName, resultType))
		} else if len(q.Params) == 1 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s, void (*cb)(%s*, void*), void *ctx);\n",
				funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle), resultType))
		} else {
			paramsType := g.typeName(q.Name + "Params")
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params, void (*cb)(%s*, void*), void *ctx);\n",
				funcName, paramsType, resultType))
		}

	case "exec":
		if len(q.Params) == 0 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db);\n\n", funcName))
		} else if len(q.Params) == 1 {
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s);\n\n",
				funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle)))
		} else {
			paramsType := g.typeName(q.Name + "Params")
			g.out.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params);\n\n", funcName, paramsType))
		}
	}
}

// generateImplementation produces the .c file
func (g *Generator) generateImplementation(headerFile string) {
	g.implOut.WriteString("// Generated from SQL - do not edit\n\n")
	g.implOut.WriteString("#include <stdlib.h>\n")
	g.implOut.WriteString("#include <string.h>\n")
	g.implOut.WriteString(fmt.Sprintf("#include \"%s\"\n\n", filepath.Base(headerFile)))

	for _, q := range g.queries {
		g.generateFunctionImpl(q)
	}
}

// generateFunctionImpl generates a function implementation
func (g *Generator) generateFunctionImpl(q Query) {
	funcName := g.funcName(q.Name)
	resultType := g.typeName(q.Table)

	// Escape SQL for C string
	sqlStr := strings.TrimSpace(escapeCString(q.SQL))

	// Function comment
	g.implOut.WriteString(fmt.Sprintf("// %s :%s\n", q.Name, q.ReturnType))

	switch q.ReturnType {
	case "one":
		g.generateOneImpl(q, funcName, resultType, sqlStr)
	case "many":
		g.generateManyImpl(q, funcName, resultType, sqlStr)
	case "exec":
		g.generateExecImpl(q, funcName, sqlStr)
	}
}

func (g *Generator) generateOneImpl(q Query, funcName, resultType, sqlStr string) {
	// Function signature
	if len(q.Params) == 0 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, void (*cb)(%s*, void*), void *ctx) {\n", funcName, resultType))
	} else if len(q.Params) == 1 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s, void (*cb)(%s*, void*), void *ctx) {\n",
			funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle), resultType))
	} else {
		paramsType := g.typeName(q.Name + "Params")
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params, void (*cb)(%s*, void*), void *ctx) {\n",
			funcName, paramsType, resultType))
	}

	g.implOut.WriteString(fmt.Sprintf("    const char *sql = %s;\n", sqlStr))
	g.implOut.WriteString("    sqlite3_stmt *stmt;\n")
	g.implOut.WriteString("    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);\n")
	g.implOut.WriteString("    if (rc != SQLITE_OK) return rc;\n\n")

	// Bind parameters
	g.generateBindParams(q)

	g.implOut.WriteString("    rc = sqlite3_step(stmt);\n")
	g.implOut.WriteString("    if (rc == SQLITE_ROW) {\n")
	g.implOut.WriteString(fmt.Sprintf("        %s result = {0};\n", resultType))

	// Extract columns
	g.generateExtractColumns(q.Columns)

	g.implOut.WriteString("        if (cb) cb(&result, ctx);\n")
	g.implOut.WriteString("        rc = SQLITE_OK;\n")
	g.implOut.WriteString("    } else if (rc == SQLITE_DONE) {\n")
	g.implOut.WriteString("        rc = SQLITE_NOTFOUND;\n")
	g.implOut.WriteString("    }\n")
	g.implOut.WriteString("    sqlite3_finalize(stmt);\n")
	g.implOut.WriteString("    return rc;\n")
	g.implOut.WriteString("}\n\n")
}

func (g *Generator) generateManyImpl(q Query, funcName, resultType, sqlStr string) {
	// Function signature
	if len(q.Params) == 0 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, void (*cb)(%s*, void*), void *ctx) {\n", funcName, resultType))
	} else if len(q.Params) == 1 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s, void (*cb)(%s*, void*), void *ctx) {\n",
			funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle), resultType))
	} else {
		paramsType := g.typeName(q.Name + "Params")
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params, void (*cb)(%s*, void*), void *ctx) {\n",
			funcName, paramsType, resultType))
	}

	g.implOut.WriteString(fmt.Sprintf("    const char *sql = %s;\n", sqlStr))
	g.implOut.WriteString("    sqlite3_stmt *stmt;\n")
	g.implOut.WriteString("    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);\n")
	g.implOut.WriteString("    if (rc != SQLITE_OK) return rc;\n\n")

	// Bind parameters
	g.generateBindParams(q)

	g.implOut.WriteString("    size_t capacity = 16;\n")
	g.implOut.WriteString("    size_t n = 0;\n")
	g.implOut.WriteString(fmt.Sprintf("    %s *arr = malloc(capacity * sizeof(%s));\n", resultType, resultType))
	g.implOut.WriteString("    if (arr == NULL) { sqlite3_finalize(stmt); return SQLITE_NOMEM; }\n\n")

	g.implOut.WriteString("    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {\n")
	g.implOut.WriteString(fmt.Sprintf("        %s result = {0};\n", resultType))

	// Extract columns
	g.generateExtractColumns(q.Columns)

	g.implOut.WriteString("        if (cb) cb(&result, ctx);\n")
	g.implOut.WriteString("    }\n\n")

	g.implOut.WriteString("    sqlite3_finalize(stmt);\n")
	g.implOut.WriteString("    if (rc == SQLITE_DONE) rc = SQLITE_OK;\n")
	g.implOut.WriteString("    return rc;\n")
	g.implOut.WriteString("}\n\n")
}

func (g *Generator) generateExecImpl(q Query, funcName, sqlStr string) {
	// Function signature
	if len(q.Params) == 0 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db) {\n", funcName))
	} else if len(q.Params) == 1 {
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s %s) {\n",
			funcName, q.Params[0].Type, g.applyStyle(q.Params[0].Name, g.fieldStyle)))
	} else {
		paramsType := g.typeName(q.Name + "Params")
		g.implOut.WriteString(fmt.Sprintf("int %s(sqlite3 *db, %s *params) {\n", funcName, paramsType))
	}

	g.implOut.WriteString(fmt.Sprintf("    const char *sql = %s;\n", sqlStr))
	g.implOut.WriteString("    sqlite3_stmt *stmt;\n")
	g.implOut.WriteString("    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);\n")
	g.implOut.WriteString("    if (rc != SQLITE_OK) return rc;\n\n")

	// Bind parameters
	g.generateBindParams(q)

	g.implOut.WriteString("    rc = sqlite3_step(stmt);\n")
	g.implOut.WriteString("    sqlite3_finalize(stmt);\n")
	g.implOut.WriteString("    return (rc == SQLITE_DONE) ? SQLITE_OK : rc;\n")
	g.implOut.WriteString("}\n\n")
}

func (g *Generator) generateBindParams(q Query) {
	for i, p := range q.Params {
		idx := i + 1
		var paramAccess string
		if len(q.Params) == 1 {
			paramAccess = g.applyStyle(p.Name, g.fieldStyle)
		} else {
			paramAccess = fmt.Sprintf("params->%s", g.applyStyle(p.Name, g.fieldStyle))
		}

		switch p.Type {
		case "sql_int64":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_int64(stmt, %d, %s);\n", idx, paramAccess))
		case "sql_int":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_int(stmt, %d, %s);\n", idx, paramAccess))
		case "sql_text":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_text(stmt, %d, (char*)%s.data, %s.len, SQLITE_STATIC);\n",
				idx, paramAccess, paramAccess))
		case "sql_double":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_double(stmt, %d, %s);\n", idx, paramAccess))
		case "sql_blob":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_blob(stmt, %d, %s.data, %s.len, SQLITE_STATIC);\n",
				idx, paramAccess, paramAccess))
		case "sql_bool":
			g.implOut.WriteString(fmt.Sprintf("    sqlite3_bind_int(stmt, %d, %s ? 1 : 0);\n", idx, paramAccess))
		}
	}
	if len(q.Params) > 0 {
		g.implOut.WriteString("\n")
	}
}

func (g *Generator) generateExtractColumns(cols []Column) {
	for i, col := range cols {
		fieldName := g.applyStyle(col.Name, g.fieldStyle)
		sqlType := g.sqliteTypeToSQLType(col.Type, col.Nullable)

		switch {
		case strings.HasPrefix(sqlType, "sql_nullint"):
			g.implOut.WriteString(fmt.Sprintf("        if (sqlite3_column_type(stmt, %d) != SQLITE_NULL) {\n", i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.value = sqlite3_column_int64(stmt, %d);\n", fieldName, i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.null = false;\n", fieldName))
			g.implOut.WriteString(fmt.Sprintf("        } else { result.%s.null = true; }\n", fieldName))
		case sqlType == "sql_int64":
			g.implOut.WriteString(fmt.Sprintf("        result.%s = sqlite3_column_int64(stmt, %d);\n", fieldName, i))
		case sqlType == "sql_int":
			g.implOut.WriteString(fmt.Sprintf("        result.%s = sqlite3_column_int(stmt, %d);\n", fieldName, i))
		case strings.HasPrefix(sqlType, "sql_nulltext"):
			g.implOut.WriteString(fmt.Sprintf("        if (sqlite3_column_type(stmt, %d) != SQLITE_NULL) {\n", i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.data = sqlite3_column_text(stmt, %d);\n", fieldName, i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.len = sqlite3_column_bytes(stmt, %d);\n", fieldName, i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.null = false;\n", fieldName))
			g.implOut.WriteString(fmt.Sprintf("        } else { result.%s.null = true; }\n", fieldName))
		case sqlType == "sql_text":
			g.implOut.WriteString(fmt.Sprintf("        result.%s.data = (sql_byte*)sqlite3_column_text(stmt, %d);\n", fieldName, i))
			g.implOut.WriteString(fmt.Sprintf("        result.%s.len = sqlite3_column_bytes(stmt, %d);\n", fieldName, i))
		case strings.HasPrefix(sqlType, "sql_nulldouble"):
			g.implOut.WriteString(fmt.Sprintf("        if (sqlite3_column_type(stmt, %d) != SQLITE_NULL) {\n", i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.value = sqlite3_column_double(stmt, %d);\n", fieldName, i))
			g.implOut.WriteString(fmt.Sprintf("            result.%s.null = false;\n", fieldName))
			g.implOut.WriteString(fmt.Sprintf("        } else { result.%s.null = true; }\n", fieldName))
		case sqlType == "sql_double":
			g.implOut.WriteString(fmt.Sprintf("        result.%s = sqlite3_column_double(stmt, %d);\n", fieldName, i))
		case sqlType == "sql_bool":
			g.implOut.WriteString(fmt.Sprintf("        result.%s = sqlite3_column_int(stmt, %d) != 0;\n", fieldName, i))
		default:
			// Treat unknown as text
			g.implOut.WriteString(fmt.Sprintf("        result.%s = sqlite3_column_text(stmt, %d); // BUG\n", fieldName, i))
		}
	}
}

// Helper functions

func (g *Generator) applyStyle(s string, style NamingStyle) string {
	switch style {
	case SnakeCase:
		return toSnakeCase(s)
	case CamelCase:
		return toCamelCase(s)
	case PascalCase:
		return toPascalCase(s)
	default:
		return s
	}
}

func (g *Generator) typeName(name string) string {
	styled := g.applyStyle(name, g.structStyle)
	if g.typePrefix != "" {
		return g.typePrefix + styled
	}
	return styled
}

func (g *Generator) funcName(name string) string {
	styled := g.applyStyle(name, g.funcStyle)
	if g.funcPrefix != "" {
		return g.funcPrefix + styled
	}
	return styled
}

func toPascalCase(s string) string {
	parts := splitIdentifier(s)
	for i := range parts {
		if len(parts[i]) > 0 {
			parts[i] = strings.ToUpper(parts[i][:1]) + strings.ToLower(parts[i][1:])
		}
	}
	return strings.Join(parts, "")
}

func toCamelCase(s string) string {
	pascal := toPascalCase(s)
	if len(pascal) == 0 {
		return pascal
	}
	return strings.ToLower(pascal[:1]) + pascal[1:]
}

func toSnakeCase(s string) string {
	var result strings.Builder
	prevLower := false

	for i, r := range s {
		isUpper := r >= 'A' && r <= 'Z'
		isSeparator := r == '-' || r == '_' || r == ' '

		if isSeparator {
			if result.Len() > 0 {
				result.WriteRune('_')
			}
			prevLower = false
			continue
		}

		if i > 0 && isUpper && prevLower {
			result.WriteRune('_')
		}

		result.WriteRune(r)
		prevLower = !isUpper
	}

	return strings.ToLower(result.String())
}

func splitIdentifier(s string) []string {
	var parts []string
	var current strings.Builder

	for i, r := range s {
		isSeparator := r == '-' || r == '_' || r == ' '
		isUpper := r >= 'A' && r <= 'Z'

		if isSeparator {
			if current.Len() > 0 {
				parts = append(parts, current.String())
				current.Reset()
			}
			continue
		}

		if i > 0 && isUpper && current.Len() > 0 {
			lastChar := []rune(current.String())[current.Len()-1]
			if lastChar >= 'a' && lastChar <= 'z' {
				parts = append(parts, current.String())
				current.Reset()
			}
		}

		current.WriteRune(r)
	}

	if current.Len() > 0 {
		parts = append(parts, current.String())
	}

	return parts
}

// Ensure sorted output for determinism
func (g *Generator) sortedTableNames() []string {
	names := make([]string, 0, len(g.tables))
	for name := range g.tables {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

func escapeCString(s string) string {
	// Remove trailing newline to avoid empty last line
	s = strings.TrimSuffix(s, "\n")
	lines := strings.Split(s, "\n")
	var result strings.Builder

	for i, line := range lines {
		// Escape special characters
		escaped := strings.ReplaceAll(line, "\\", "\\\\")
		escaped = strings.ReplaceAll(escaped, "\"", "\\\"")

		result.WriteString("                      \"")
		result.WriteString(escaped)
		result.WriteString("\\n\"")

		// if i == len(lines)-1 {
		// 	result.WriteString(";")
		// }
		if i != len(lines)-1 {
			result.WriteString("\n")
		}
	}

	return result.String()
}
