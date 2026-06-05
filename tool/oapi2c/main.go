package main

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strings"

	"github.com/getkin/kin-openapi/openapi3"
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
	Spec         string `yaml:"spec"`
	Output       string `yaml:"output"`
	StructStyle  string `yaml:"struct-style"`
	FieldStyle   string `yaml:"field-style"`
	FuncStyle    string `yaml:"func-style"`
	GenJSON      bool   `yaml:"gen-json"`
	TypePrefix   string `yaml:"type-prefix"`
	FuncPrefix   string `yaml:"func-prefix"`
	UseAllocator bool   `yaml:"use-allocator"`
	Framework    string `yaml:"framework"` // "none" (default) or "ecewo"
}

// FieldInfo stores information about a struct field for JSON generation
type FieldInfo struct {
	JSONName       string // Original name from OpenAPI (used in JSON)
	FieldName      string // C field name (styled)
	CType          string // C type
	IsArray        bool
	IsRef          bool   // Is a reference to another struct
	RefType        string // The referenced struct type (if IsRef)
	RefSchemaName  string // Original schema name for the reference (for function names)
	IsString       bool
	IsInt          bool
	IsInt32        bool
	IsFloat        bool
	IsDouble       bool
	IsBool         bool
	IsEnum         bool   // Is an enum type
	EnumType       string // The enum type name (if IsEnum)
	EnumSchemaName string // Original schema name for the enum (for function names)
	CountField     string // Name of the count field for arrays
}

// EnumInfo stores information about an enum for code generation
type EnumInfo struct {
	Name     string   // Original schema name
	EnumName string   // C enum name (styled)
	Values   []string // Enum values from OpenAPI
}

// StructInfo stores information about a struct for JSON generation
type StructInfo struct {
	Name       string // Original schema name
	StructName string // C struct name (styled)
	Fields     []FieldInfo
}

type Generator struct {
	doc          *openapi3.T
	out          strings.Builder
	jsonOut      strings.Builder
	structStyle  NamingStyle
	fieldStyle   NamingStyle
	funcStyle    NamingStyle
	genJSON      bool
	typePrefix   string       // Prefix for type names (structs, enums)
	funcPrefix   string       // Prefix for function names
	useAllocator bool         // Add allocator parameter to JSON functions
	framework    string       // "none" or "ecewo"
	structs      []StructInfo // Collected struct info for JSON generation
	enums        []EnumInfo   // Collected enum info for JSON generation
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
		Spec:        "openapi.yaml",
		Output:      "api.h",
		StructStyle: "pascal",
		FieldStyle:  "snake",
		FuncStyle:   "snake",
		GenJSON:     false,
		Framework:   "none",
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
		// Apply config file values (only non-empty values)
		if fileCfg.Spec != "" {
			cfg.Spec = fileCfg.Spec
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
		if fileCfg.GenJSON {
			cfg.GenJSON = true
		}
		if fileCfg.TypePrefix != "" {
			cfg.TypePrefix = fileCfg.TypePrefix
		}
		if fileCfg.FuncPrefix != "" {
			cfg.FuncPrefix = fileCfg.FuncPrefix
		}
		if fileCfg.UseAllocator {
			cfg.UseAllocator = true
		}
		if fileCfg.Framework != "" {
			cfg.Framework = fileCfg.Framework
		}
	}

	// Define flags with config values as defaults
	configFlag := flag.String("config", "", "Path to config file (YAML)")
	specPath := flag.String("spec", cfg.Spec, "Path to OpenAPI spec file")
	output := flag.String("output", cfg.Output, "Output header file")
	structStyle := flag.String("struct-style", cfg.StructStyle, "Naming style for structs: snake, camel, pascal")
	fieldStyle := flag.String("field-style", cfg.FieldStyle, "Naming style for fields: snake, camel, pascal")
	funcStyle := flag.String("func-style", cfg.FuncStyle, "Naming style for functions: snake, camel, pascal")
	genJSON := flag.Bool("gen-json", cfg.GenJSON, "Generate JSON marshal/unmarshal functions")
	typePrefix := flag.String("type-prefix", cfg.TypePrefix, "Prefix for type names (structs, enums)")
	funcPrefix := flag.String("func-prefix", cfg.FuncPrefix, "Prefix for function names")
	useAllocator := flag.Bool("use-allocator", cfg.UseAllocator, "Add allocator parameter to JSON functions")
	framework := flag.String("framework", cfg.Framework, "Web framework: none (default) or ecewo")
	flag.Parse()

	// Suppress unused warning
	_ = configFlag

	loader := openapi3.NewLoader()
	doc, err := loader.LoadFromFile(*specPath)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error loading spec: %v\n", err)
		os.Exit(1)
	}

	if err := doc.Validate(loader.Context); err != nil {
		fmt.Fprintf(os.Stderr, "Invalid spec: %v\n", err)
		os.Exit(1)
	}

	gen := &Generator{
		doc:          doc,
		structStyle:  NamingStyle(*structStyle),
		fieldStyle:   NamingStyle(*fieldStyle),
		funcStyle:    NamingStyle(*funcStyle),
		genJSON:      *genJSON,
		typePrefix:   *typePrefix,
		funcPrefix:   *funcPrefix,
		useAllocator: *useAllocator,
		framework:    *framework,
	}

	gen.generate(*output)

	// Create output directory if needed
	if dir := filepath.Dir(*output); dir != "." && dir != "" {
		if err := os.MkdirAll(dir, 0755); err != nil {
			fmt.Fprintf(os.Stderr, "Error creating directory: %v\n", err)
			os.Exit(1)
		}
	}

	if err := os.WriteFile(*output, []byte(gen.out.String()), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "Error writing output: %v\n", err)
		os.Exit(1)
	}
	fmt.Printf("Generated %s\n", *output)

	// Generate JSON implementation file
	if *genJSON {
		jsonFile := strings.TrimSuffix(*output, filepath.Ext(*output)) + "_json.c"
		if err := os.WriteFile(jsonFile, []byte(gen.jsonOut.String()), 0644); err != nil {
			fmt.Fprintf(os.Stderr, "Error writing JSON output: %v\n", err)
			os.Exit(1)
		}
		fmt.Printf("Generated %s\n", jsonFile)
	}
}

func (g *Generator) generate(outputFile string) {
	// Header guard
	guardName := strings.ToUpper(strings.ReplaceAll(filepath.Base(outputFile), ".", "_"))
	g.out.WriteString("// Generated from OpenAPI spec - do not edit\n\n")
	g.out.WriteString(fmt.Sprintf("#ifndef %s\n", guardName))
	g.out.WriteString(fmt.Sprintf("#define %s\n\n", guardName))
	g.out.WriteString("#include <stdint.h>\n")
	g.out.WriteString("#include <stdbool.h>\n")
	g.out.WriteString("#include <stddef.h>\n\n")

	// Framework includes
	if g.framework == "ecewo" {
		g.out.WriteString("#include \"ecewo.h\"\n\n")
	}

	// Generate Allocator typedef if enabled
	if g.useAllocator && g.genJSON {
		g.generateAllocatorTypedef()

		// Generate arena adapter for ecewo
		if g.framework == "ecewo" {
			g.generateArenaAllocator()
		}
	}

	// Generate enums first (before structs that may use them)
	g.out.WriteString("// ============ Enums ============\n\n")
	g.generateEnums()

	// Forward declare all structs first (for circular references)
	g.out.WriteString("// Struct forward declarations\n")
	g.generateForwardDeclarations()

	// Generate structs from schemas
	g.out.WriteString("// ============ Structs ============\n\n")
	g.generateStructs()

	// Generate JSON function declarations
	if g.genJSON {
		g.out.WriteString("// ============ JSON Functions ============\n\n")
		g.generateJSONDeclarations()
	}

	// Generate handler signatures from paths (framework-specific)
	if g.framework == "ecewo" {
		g.out.WriteString("// ============ Handlers (ecewo) ============\n\n")
		g.generateHandlers()

		g.out.WriteString("// ============ Routes (ecewo) ============\n\n")
		g.generateRoutes()
	}

	// Close header guard
	g.out.WriteString(fmt.Sprintf("#endif // %s\n", guardName))

	// Generate JSON implementation file
	if g.genJSON {
		g.generateJSONImplementation(outputFile)
	}
}

func (g *Generator) generateAllocatorTypedef() {
	allocName := g.typeName("Allocator")
	g.out.WriteString("// ============ Allocator ============\n\n")
	g.out.WriteString("// Allocator interface - pass NULL to use malloc/free\n")
	g.out.WriteString("typedef struct {\n")
	g.out.WriteString("    void* (*alloc)(void *ctx, size_t size);\n")
	g.out.WriteString("    void  (*free)(void *ctx, void *ptr);  // Can be NULL for arenas\n")
	g.out.WriteString("    void *ctx;\n")
	g.out.WriteString(fmt.Sprintf("} %s;\n\n", allocName))
}

func (g *Generator) generateArenaAllocator() {
	allocName := g.typeName("Allocator")
	funcPrefix := g.funcPrefix

	g.out.WriteString("// Arena to Allocator adapter for ecewo\n")
	g.out.WriteString(fmt.Sprintf("static inline void *%sarena_alloc_fn(void *ctx, size_t size) {\n", funcPrefix))
	g.out.WriteString("    return arena_alloc((Arena *)ctx, size);\n")
	g.out.WriteString("}\n\n")

	g.out.WriteString(fmt.Sprintf("static inline %s %sarena_allocator(Arena *arena) {\n", allocName, funcPrefix))
	g.out.WriteString(fmt.Sprintf("    return (%s){ .alloc = %sarena_alloc_fn, .free = NULL, .ctx = arena };\n", allocName, funcPrefix))
	g.out.WriteString("}\n\n")
}

func (g *Generator) generateForwardDeclarations() {
	names := g.sortedSchemaNames()
	for _, name := range names {
		schema := g.doc.Components.Schemas[name]
		if schema.Value != nil && schema.Value.Type.Is("object") {
			structName := g.typeName(name)
			g.out.WriteString(fmt.Sprintf("typedef struct %s %s;\n", structName, structName))
		}
	}
	g.out.WriteString("\n")
}

func (g *Generator) generateEnums() {
	names := g.sortedSchemaNames()
	for _, name := range names {
		schema := g.doc.Components.Schemas[name]
		if schema.Value == nil {
			continue
		}
		// Check if it's an enum (string type with enum values)
		if schema.Value.Type.Is("string") && len(schema.Value.Enum) > 0 {
			g.generateEnum(name, schema.Value)
		}
	}
}

func (g *Generator) generateEnum(name string, schema *openapi3.Schema) {
	enumName := g.typeName(name)

	// Schema description as comment
	if schema.Description != "" {
		g.out.WriteString(fmt.Sprintf("// %s\n", wrapComment(schema.Description)))
	}

	g.out.WriteString(fmt.Sprintf("typedef enum {\n"))

	// Collect enum values
	var values []string
	for _, v := range schema.Enum {
		if s, ok := v.(string); ok {
			values = append(values, s)
		}
	}

	// Generate enum constants
	for i, value := range values {
		constName := g.enumConstantName(enumName, value)
		if i < len(values)-1 {
			g.out.WriteString(fmt.Sprintf("    %s,\n", constName))
		} else {
			g.out.WriteString(fmt.Sprintf("    %s\n", constName))
		}
	}

	g.out.WriteString(fmt.Sprintf("} %s;\n\n", enumName))

	// Store enum info for JSON generation
	g.enums = append(g.enums, EnumInfo{
		Name:     name,
		EnumName: enumName,
		Values:   values,
	})
}

func (g *Generator) enumConstantName(enumName, value string) string {
	// Generate constant name: ENUM_NAME_VALUE
	return strings.ToUpper(toSnakeCase(enumName)) + "_" + strings.ToUpper(toSnakeCase(value))
}

func (g *Generator) isEnumRef(ref string) bool {
	parts := strings.Split(ref, "/")
	if len(parts) == 0 {
		return false
	}
	schemaName := parts[len(parts)-1]
	schema := g.doc.Components.Schemas[schemaName]
	if schema == nil || schema.Value == nil {
		return false
	}
	return schema.Value.Type.Is("string") && len(schema.Value.Enum) > 0
}

func (g *Generator) isEnumSchema(schema *openapi3.Schema) bool {
	return schema != nil && schema.Type.Is("string") && len(schema.Enum) > 0
}

func (g *Generator) generateStructs() {
	names := g.sortedSchemaNames()
	for _, name := range names {
		schema := g.doc.Components.Schemas[name]
		if schema.Value == nil {
			continue
		}
		if schema.Value.Type.Is("object") {
			g.generateStruct(name, schema.Value)
		}
	}
}

func (g *Generator) generateStruct(name string, schema *openapi3.Schema) {
	structName := g.typeName(name)

	// Schema description as comment
	if schema.Description != "" {
		g.out.WriteString(fmt.Sprintf("// %s\n", wrapComment(schema.Description)))
	}

	g.out.WriteString(fmt.Sprintf("struct %s {\n", structName))

	// Collect struct info for JSON generation
	structInfo := StructInfo{
		Name:       name,
		StructName: structName,
		Fields:     []FieldInfo{},
	}

	// Sort properties for deterministic output
	propNames := g.sortedPropertyNames(schema.Properties)

	for _, propName := range propNames {
		prop := schema.Properties[propName]
		fieldInfo := g.generateField(propName, prop)
		structInfo.Fields = append(structInfo.Fields, fieldInfo...)
	}

	g.out.WriteString("};\n\n")

	// Store struct info for JSON generation
	g.structs = append(g.structs, structInfo)
}

func (g *Generator) generateField(propName string, propRef *openapi3.SchemaRef) []FieldInfo {
	fieldName := g.applyStyle(propName, g.fieldStyle)

	// Field description as comment
	if propRef.Value != nil && propRef.Value.Description != "" {
		g.out.WriteString(fmt.Sprintf("    // %s\n", wrapComment(propRef.Value.Description)))
	}

	// Check if it's a $ref to another schema
	if propRef.Ref != "" {
		refType := g.resolveRefType(propRef.Ref)
		schemaName := g.extractSchemaName(propRef.Ref)
		// Check if it's an enum reference
		if g.isEnumRef(propRef.Ref) {
			g.out.WriteString(fmt.Sprintf("    %s %s;\n", refType, fieldName))
			return []FieldInfo{{
				JSONName:       propName,
				FieldName:      fieldName,
				CType:          refType,
				IsEnum:         true,
				EnumType:       refType,
				EnumSchemaName: schemaName,
			}}
		}
		// It's a struct reference
		g.out.WriteString(fmt.Sprintf("    %s *%s;\n", refType, fieldName))
		return []FieldInfo{{
			JSONName:      propName,
			FieldName:     fieldName,
			CType:         refType + "*",
			IsRef:         true,
			RefType:       refType,
			RefSchemaName: schemaName,
		}}
	}

	if propRef.Value == nil {
		g.out.WriteString(fmt.Sprintf("    void *%s;\n", fieldName))
		return []FieldInfo{{JSONName: propName, FieldName: fieldName, CType: "void*"}}
	}

	schema := propRef.Value

	// Handle arrays specially - generate pointer + count
	if schema.Type.Is("array") {
		return g.generateArrayField(propName, fieldName, schema)
	}

	// Handle nested objects
	if schema.Type.Is("object") {
		g.out.WriteString(fmt.Sprintf("    void *%s;\n", fieldName))
		return []FieldInfo{{JSONName: propName, FieldName: fieldName, CType: "void*"}}
	}

	// Handle inline enums (string with enum values)
	if g.isEnumSchema(schema) {
		// For inline enums, we treat them as int with a comment about valid values
		g.out.WriteString(fmt.Sprintf("    int %s; // enum: %v\n", fieldName, schema.Enum))
		return []FieldInfo{{
			JSONName:  propName,
			FieldName: fieldName,
			CType:     "int",
			IsInt:     true,
		}}
	}

	cType := g.openAPITypeToCType(propRef)
	g.out.WriteString(fmt.Sprintf("    %s %s;\n", cType, fieldName))

	field := FieldInfo{
		JSONName:  propName,
		FieldName: fieldName,
		CType:     cType,
	}

	// Determine type for JSON generation
	switch {
	case schema.Type.Is("string"):
		field.IsString = true
	case schema.Type.Is("integer"):
		field.IsInt = true
		if schema.Format == "int32" {
			field.IsInt32 = true
		}
	case schema.Type.Is("number"):
		if schema.Format == "float" {
			field.IsFloat = true
		} else {
			field.IsDouble = true
		}
	case schema.Type.Is("boolean"):
		field.IsBool = true
	}

	return []FieldInfo{field}
}

func (g *Generator) generateArrayField(propName, fieldName string, schema *openapi3.Schema) []FieldInfo {
	var itemType string
	var isRef bool
	var refType string
	var refSchemaName string
	var isEnum bool
	var enumType string
	var enumSchemaName string
	var isString, isInt, isInt32, isFloat, isDouble, isBool bool

	if schema.Items != nil {
		if schema.Items.Ref != "" {
			itemType = g.resolveRefType(schema.Items.Ref)
			schemaName := g.extractSchemaName(schema.Items.Ref)
			// Check if it's an enum reference
			if g.isEnumRef(schema.Items.Ref) {
				isEnum = true
				enumType = itemType
				enumSchemaName = schemaName
			} else {
				isRef = true
				refType = itemType
				refSchemaName = schemaName
			}
		} else if schema.Items.Value != nil {
			itemSchema := schema.Items.Value
			// Check for inline enum in array items
			if g.isEnumSchema(itemSchema) {
				itemType = "int"
				isInt = true
			} else {
				itemType = g.openAPITypeToCType(schema.Items)
				switch {
				case itemSchema.Type.Is("string"):
					isString = true
				case itemSchema.Type.Is("integer"):
					isInt = true
					if itemSchema.Format == "int32" {
						isInt32 = true
					}
				case itemSchema.Type.Is("number"):
					if itemSchema.Format == "float" {
						isFloat = true
					} else {
						isDouble = true
					}
				case itemSchema.Type.Is("boolean"):
					isBool = true
				}
			}
		} else {
			itemType = "void"
		}
	} else {
		itemType = "void"
	}

	// Generate pointer + count fields
	countFieldName := g.applyStyle(propName+"_count", g.fieldStyle)
	g.out.WriteString(fmt.Sprintf("    %s *%s;\n", itemType, fieldName))
	g.out.WriteString(fmt.Sprintf("    size_t %s;\n", countFieldName))

	return []FieldInfo{{
		JSONName:       propName,
		FieldName:      fieldName,
		CType:          itemType + "*",
		IsArray:        true,
		IsRef:          isRef,
		RefType:        refType,
		RefSchemaName:  refSchemaName,
		IsEnum:         isEnum,
		EnumType:       enumType,
		EnumSchemaName: enumSchemaName,
		IsString:       isString,
		IsInt:          isInt,
		IsInt32:        isInt32,
		IsFloat:        isFloat,
		IsDouble:       isDouble,
		IsBool:         isBool,
		CountField:     countFieldName,
	}}
}

func (g *Generator) resolveRefType(ref string) string {
	parts := strings.Split(ref, "/")
	if len(parts) > 0 {
		schemaName := parts[len(parts)-1]
		return g.typeName(schemaName)
	}
	return "void"
}

func (g *Generator) extractSchemaName(ref string) string {
	parts := strings.Split(ref, "/")
	if len(parts) > 0 {
		return parts[len(parts)-1]
	}
	return ""
}

func (g *Generator) openAPITypeToCType(schemaRef *openapi3.SchemaRef) string {
	if schemaRef.Ref != "" {
		return g.resolveRefType(schemaRef.Ref) + "*"
	}

	schema := schemaRef.Value
	if schema == nil {
		return "void*"
	}

	nullable := schema.Nullable

	switch {
	case schema.Type.Is("string"):
		return "char*"

	case schema.Type.Is("integer"):
		baseType := "int64_t"
		switch schema.Format {
		case "int32":
			baseType = "int32_t"
		case "int64":
			baseType = "int64_t"
		}
		if nullable {
			return baseType + "*"
		}
		return baseType

	case schema.Type.Is("number"):
		baseType := "double"
		if schema.Format == "float" {
			baseType = "float"
		}
		if nullable {
			return baseType + "*"
		}
		return baseType

	case schema.Type.Is("boolean"):
		if nullable {
			return "bool*"
		}
		return "bool"

	case schema.Type.Is("array"):
		if schema.Items != nil {
			itemType := g.openAPITypeToCType(schema.Items)
			return itemType + "*"
		}
		return "void*"

	case schema.Type.Is("object"):
		return "void*"

	default:
		return "void*"
	}
}

func (g *Generator) generateJSONDeclarations() {
	allocType := g.typeName("Allocator")

	// Enum string conversion functions
	for _, e := range g.enums {
		funcName := g.funcName(e.Name)
		g.out.WriteString(fmt.Sprintf("// String conversion for %s\n", e.EnumName))
		g.out.WriteString(fmt.Sprintf("const char *%s_to_string(%s val);\n", funcName, e.EnumName))
		g.out.WriteString(fmt.Sprintf("%s %s_from_string(const char *str);\n\n", e.EnumName, funcName))
	}

	// Struct JSON functions
	for _, s := range g.structs {
		funcName := g.funcName(s.Name)
		g.out.WriteString(fmt.Sprintf("// JSON serialization for %s\n", s.StructName))
		if g.useAllocator {
			g.out.WriteString(fmt.Sprintf("char *%s_to_json(%s *alloc, %s *obj);\n", funcName, allocType, s.StructName))
			g.out.WriteString(fmt.Sprintf("int %s_from_json(%s *alloc, const char *json, %s *obj);\n", funcName, allocType, s.StructName))
			g.out.WriteString(fmt.Sprintf("void %s_free(%s *alloc, %s *obj);\n\n", funcName, allocType, s.StructName))
		} else {
			g.out.WriteString(fmt.Sprintf("char *%s_to_json(%s *obj);\n", funcName, s.StructName))
			g.out.WriteString(fmt.Sprintf("int %s_from_json(const char *json, %s *obj);\n", funcName, s.StructName))
			g.out.WriteString(fmt.Sprintf("void %s_free(%s *obj);\n\n", funcName, s.StructName))
		}
	}
}

func (g *Generator) generateJSONImplementation(headerFile string) {
	allocType := g.typeName("Allocator")

	g.jsonOut.WriteString("// Generated from OpenAPI spec - do not edit\n\n")
	g.jsonOut.WriteString("#include <stdlib.h>\n")
	g.jsonOut.WriteString("#include <string.h>\n")
	g.jsonOut.WriteString("#include <stdio.h>\n\n")
	g.jsonOut.WriteString(fmt.Sprintf("#include \"%s\"\n\n", filepath.Base(headerFile)))

	// Generate helper functions
	g.generateJSONHelpers()

	// Generate enum conversion functions
	for _, e := range g.enums {
		g.generateEnumToString(e)
		g.generateEnumFromString(e)
	}

	// Generate forward declarations
	g.jsonOut.WriteString("// Forward declarations\n")
	for _, s := range g.structs {
		funcName := g.funcName(s.Name)
		if g.useAllocator {
			g.jsonOut.WriteString(fmt.Sprintf("static const char *%s_parse(%s *alloc, const char *json, %s *obj);\n", funcName, allocType, s.StructName))
			g.jsonOut.WriteString(fmt.Sprintf("static int %s_write(%s *alloc, char *buf, size_t size, %s *obj);\n", funcName, allocType, s.StructName))
		} else {
			g.jsonOut.WriteString(fmt.Sprintf("static const char *%s_parse(const char *json, %s *obj);\n", funcName, s.StructName))
			g.jsonOut.WriteString(fmt.Sprintf("static int %s_write(char *buf, size_t size, %s *obj);\n", funcName, s.StructName))
		}
	}
	g.jsonOut.WriteString("\n")

	for _, s := range g.structs {
		g.generateStructToJSON(s)
		g.generateStructFromJSON(s)
		g.generateStructFree(s)
	}
}

func (g *Generator) generateEnumToString(e EnumInfo) {
	funcName := g.funcName(e.Name)
	g.jsonOut.WriteString(fmt.Sprintf("const char *%s_to_string(%s val) {\n", funcName, e.EnumName))
	g.jsonOut.WriteString("    switch (val) {\n")
	for _, value := range e.Values {
		constName := g.enumConstantName(e.EnumName, value)
		g.jsonOut.WriteString(fmt.Sprintf("        case %s: return \"%s\";\n", constName, value))
	}
	g.jsonOut.WriteString("        default: return NULL;\n")
	g.jsonOut.WriteString("    }\n")
	g.jsonOut.WriteString("}\n\n")
}

func (g *Generator) generateEnumFromString(e EnumInfo) {
	funcName := g.funcName(e.Name)
	g.jsonOut.WriteString(fmt.Sprintf("%s %s_from_string(const char *str) {\n", e.EnumName, funcName))
	g.jsonOut.WriteString("    if (str == NULL) return 0;\n")
	for _, value := range e.Values {
		constName := g.enumConstantName(e.EnumName, value)
		g.jsonOut.WriteString(fmt.Sprintf("    if (strcmp(str, \"%s\") == 0) return %s;\n", value, constName))
	}
	g.jsonOut.WriteString("    return 0;\n")
	g.jsonOut.WriteString("}\n\n")
}

func (g *Generator) generateJSONHelpers() {
	allocType := g.typeName("Allocator")

	g.jsonOut.WriteString("// ============ JSON Helper Functions ============\n\n")

	// Allocator helper macros/functions if enabled
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf(`// Allocator helpers - use allocator if provided, otherwise malloc/free
static void *json_alloc(%s *alloc, size_t size) {
    if (alloc && alloc->alloc) return alloc->alloc(alloc->ctx, size);
    return malloc(size);
}

static void *json_calloc(%s *alloc, size_t count, size_t size) {
    size_t total = count * size;
    void *ptr = json_alloc(alloc, total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

static void json_free(%s *alloc, void *ptr) {
    if (ptr == NULL) return;
    if (alloc && alloc->free) { alloc->free(alloc->ctx, ptr); return; }
    if (alloc && alloc->alloc) return; // Arena without free - don't call free()
    free(ptr);
}

`, allocType, allocType, allocType))
	}

	g.jsonOut.WriteString(`static const char *skip_ws(const char *p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static const char *skip_value(const char *p) {
    p = skip_ws(p);
    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p+1)) p++;
            p++;
        }
        if (*p == '"') p++;
    } else if (*p == '{') {
        int depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '{') depth++;
            else if (*p == '}') depth--;
            else if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p+1)) p++;
                    p++;
                }
            }
            if (*p) p++;
        }
    } else if (*p == '[') {
        int depth = 1;
        p++;
        while (*p && depth > 0) {
            if (*p == '[') depth++;
            else if (*p == ']') depth--;
            else if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p+1)) p++;
                    p++;
                }
            }
            if (*p) p++;
        }
    } else {
        while (*p && *p != ',' && *p != '}' && *p != ']') p++;
    }
    return p;
}

static const char *find_key(const char *json, const char *key) {
    const char *p = skip_ws(json);
    if (*p != '{') return NULL;
    p = skip_ws(p + 1);

    size_t keylen = strlen(key);
    while (*p && *p != '}') {
        if (*p != '"') return NULL;
        p++;
        const char *kstart = p;
        while (*p && *p != '"') p++;
        size_t klen = p - kstart;
        p++;
        p = skip_ws(p);
        if (*p != ':') return NULL;
        p = skip_ws(p + 1);

        if (klen == keylen && memcmp(kstart, key, keylen) == 0) {
            return p;
        }
        p = skip_value(p);
        p = skip_ws(p);
        if (*p == ',') p = skip_ws(p + 1);
    }
    return NULL;
}

`)

	// parse_string with allocator support
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf(`static char *parse_string(%s *alloc, const char *p, const char **endp) {
    p = skip_ws(p);
    if (*p != '"') { *endp = p; return NULL; }
    p++;
    const char *start = p;
    size_t len = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && *(p+1)) { p += 2; len++; }
        else { p++; len++; }
    }
    char *str = json_alloc(alloc, len + 1);
    if (!str) { *endp = p; return NULL; }
    const char *s = start;
    char *d = str;
    while (s < p) {
        if (*s == '\\' && *(s+1)) {
            s++;
            switch (*s) {
                case 'n': *d++ = '\n'; break;
                case 't': *d++ = '\t'; break;
                case 'r': *d++ = '\r'; break;
                case '"': *d++ = '"'; break;
                case '\\': *d++ = '\\'; break;
                default: *d++ = *s; break;
            }
            s++;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    *endp = (*p == '"') ? p + 1 : p;
    return str;
}

`, allocType))
	} else {
		g.jsonOut.WriteString(`static char *parse_string(const char *p, const char **endp) {
    p = skip_ws(p);
    if (*p != '"') { *endp = p; return NULL; }
    p++;
    const char *start = p;
    size_t len = 0;
    while (*p && *p != '"') {
        if (*p == '\\' && *(p+1)) { p += 2; len++; }
        else { p++; len++; }
    }
    char *str = malloc(len + 1);
    if (!str) { *endp = p; return NULL; }
    const char *s = start;
    char *d = str;
    while (s < p) {
        if (*s == '\\' && *(s+1)) {
            s++;
            switch (*s) {
                case 'n': *d++ = '\n'; break;
                case 't': *d++ = '\t'; break;
                case 'r': *d++ = '\r'; break;
                case '"': *d++ = '"'; break;
                case '\\': *d++ = '\\'; break;
                default: *d++ = *s; break;
            }
            s++;
        } else {
            *d++ = *s++;
        }
    }
    *d = '\0';
    *endp = (*p == '"') ? p + 1 : p;
    return str;
}

`)
	}

	g.jsonOut.WriteString(`static int64_t parse_int(const char *p, const char **endp) {
    p = skip_ws(p);
    char *end;
    int64_t val = strtoll(p, &end, 10);
    *endp = end;
    return val;
}

static double parse_double(const char *p, const char **endp) {
    p = skip_ws(p);
    char *end;
    double val = strtod(p, &end);
    *endp = end;
    return val;
}

static bool parse_bool(const char *p, const char **endp) {
    p = skip_ws(p);
    if (strncmp(p, "true", 4) == 0) { *endp = p + 4; return true; }
    if (strncmp(p, "false", 5) == 0) { *endp = p + 5; return false; }
    *endp = p;
    return false;
}

static size_t count_array(const char *p) {
    p = skip_ws(p);
    if (*p != '[') return 0;
    p = skip_ws(p + 1);
    if (*p == ']') return 0;
    size_t count = 1;
    while (*p && *p != ']') {
        if (*p == ',' ) count++;
        else if (*p == '"') {
            p++;
            while (*p && *p != '"') {
                if (*p == '\\' && *(p+1)) p++;
                p++;
            }
        } else if (*p == '{' || *p == '[') {
            p = skip_value(p) - 1;
        }
        if (*p) p++;
    }
    return count;
}

static const char *array_first(const char *p) {
    p = skip_ws(p);
    if (*p != '[') return NULL;
    p = skip_ws(p + 1);
    if (*p == ']') return NULL;
    return p;
}

static const char *array_next(const char *p) {
    p = skip_value(p);
    p = skip_ws(p);
    if (*p == ',') return skip_ws(p + 1);
    return NULL;
}

static int write_escaped(char *buf, size_t size, const char *str) {
    if (!str) return snprintf(buf, size, "null");
    int len = 0;
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"");
    for (const char *p = str; *p; p++) {
        switch (*p) {
            case '"':  len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\\""); break;
            case '\\': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\\\"); break;
            case '\n': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\n"); break;
            case '\r': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\r"); break;
            case '\t': len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\\t"); break;
            default:   len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "%c", *p); break;
        }
    }
    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, "\"");
    return len;
}

`)
}

func (g *Generator) generateStructToJSON(s StructInfo) {
	funcName := g.funcName(s.Name)
	allocType := g.typeName("Allocator")

	// Internal write function (returns bytes written)
	// Note: _write doesn't need allocator since it just calculates/writes to provided buffer
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf("static int %s_write(%s *alloc, char *buf, size_t size, %s *obj) {\n", funcName, allocType, s.StructName))
		g.jsonOut.WriteString("    (void)alloc; // unused in write, but kept for consistent signature\n")
	} else {
		g.jsonOut.WriteString(fmt.Sprintf("static int %s_write(char *buf, size_t size, %s *obj) {\n", funcName, s.StructName))
	}
	g.jsonOut.WriteString("    if (obj == NULL) return snprintf(buf, size, \"null\");\n")
	g.jsonOut.WriteString("    int len = 0;\n")
	g.jsonOut.WriteString("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"{\");\n")

	for i, f := range s.Fields {
		comma := ""
		if i > 0 {
			comma = ","
		}

		if f.IsArray {
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":[\");\n", comma, f.JSONName))
			g.jsonOut.WriteString(fmt.Sprintf("    for (size_t i = 0; i < obj->%s; i++) {\n", f.CountField))
			g.jsonOut.WriteString("        if (i > 0) len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \",\");\n")

			if f.IsRef {
				refFuncName := g.funcName(f.RefSchemaName)
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("        len += %s_write(alloc, buf + len, size > (size_t)len ? size - len : 0, &obj->%s[i]);\n", refFuncName, f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("        len += %s_write(buf + len, size > (size_t)len ? size - len : 0, &obj->%s[i]);\n", refFuncName, f.FieldName))
				}
			} else if f.IsEnum {
				enumFuncName := g.funcName(f.EnumSchemaName)
				g.jsonOut.WriteString(fmt.Sprintf("        len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, %s_to_string(obj->%s[i]));\n", enumFuncName, f.FieldName))
			} else if f.IsString {
				g.jsonOut.WriteString(fmt.Sprintf("        len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->%s[i]);\n", f.FieldName))
			} else if f.IsInt {
				g.jsonOut.WriteString(fmt.Sprintf("        len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%%lld\", (long long)obj->%s[i]);\n", f.FieldName))
			} else if f.IsFloat || f.IsDouble {
				g.jsonOut.WriteString(fmt.Sprintf("        len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%%g\", (double)obj->%s[i]);\n", f.FieldName))
			} else if f.IsBool {
				g.jsonOut.WriteString(fmt.Sprintf("        len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%%s\", obj->%s[i] ? \"true\" : \"false\");\n", f.FieldName))
			}

			g.jsonOut.WriteString("    }\n")
			g.jsonOut.WriteString("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"]\");\n")
		} else if f.IsEnum {
			enumFuncName := g.funcName(f.EnumSchemaName)
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":\");\n", comma, f.JSONName))
			g.jsonOut.WriteString(fmt.Sprintf("    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, %s_to_string(obj->%s));\n", enumFuncName, f.FieldName))
		} else if f.IsRef {
			refFuncName := g.funcName(f.RefSchemaName)
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":\");\n", comma, f.JSONName))
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("    len += %s_write(alloc, buf + len, size > (size_t)len ? size - len : 0, obj->%s);\n", refFuncName, f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("    len += %s_write(buf + len, size > (size_t)len ? size - len : 0, obj->%s);\n", refFuncName, f.FieldName))
			}
		} else if f.IsString {
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":\");\n", comma, f.JSONName))
			g.jsonOut.WriteString(fmt.Sprintf("    len += write_escaped(buf + len, size > (size_t)len ? size - len : 0, obj->%s);\n", f.FieldName))
		} else if f.IsInt {
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":%%lld\", (long long)obj->%s);\n", comma, f.JSONName, f.FieldName))
		} else if f.IsFloat || f.IsDouble {
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":%%g\", (double)obj->%s);\n", comma, f.JSONName, f.FieldName))
		} else if f.IsBool {
			g.jsonOut.WriteString(fmt.Sprintf("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"%s\\\"%s\\\":%%s\", obj->%s ? \"true\" : \"false\");\n", comma, f.JSONName, f.FieldName))
		}
	}

	g.jsonOut.WriteString("    len += snprintf(buf + len, size > (size_t)len ? size - len : 0, \"}\");\n")
	g.jsonOut.WriteString("    return len;\n")
	g.jsonOut.WriteString("}\n\n")

	// Public function
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf("char *%s_to_json(%s *alloc, %s *obj) {\n", funcName, allocType, s.StructName))
		g.jsonOut.WriteString(fmt.Sprintf("    int len = %s_write(alloc, NULL, 0, obj);\n", funcName))
		g.jsonOut.WriteString("    char *buf = json_alloc(alloc, len + 1);\n")
		g.jsonOut.WriteString("    if (buf == NULL) return NULL;\n")
		g.jsonOut.WriteString(fmt.Sprintf("    %s_write(alloc, buf, len + 1, obj);\n", funcName))
		g.jsonOut.WriteString("    return buf;\n")
		g.jsonOut.WriteString("}\n\n")
	} else {
		g.jsonOut.WriteString(fmt.Sprintf("char *%s_to_json(%s *obj) {\n", funcName, s.StructName))
		g.jsonOut.WriteString(fmt.Sprintf("    int len = %s_write(NULL, 0, obj);\n", funcName))
		g.jsonOut.WriteString("    char *buf = malloc(len + 1);\n")
		g.jsonOut.WriteString("    if (buf == NULL) return NULL;\n")
		g.jsonOut.WriteString(fmt.Sprintf("    %s_write(buf, len + 1, obj);\n", funcName))
		g.jsonOut.WriteString("    return buf;\n")
		g.jsonOut.WriteString("}\n\n")
	}
}

func (g *Generator) generateStructFromJSON(s StructInfo) {
	funcName := g.funcName(s.Name)
	allocType := g.typeName("Allocator")

	// Internal parse function
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf("static const char *%s_parse(%s *alloc, const char *json, %s *obj) {\n", funcName, allocType, s.StructName))
	} else {
		g.jsonOut.WriteString(fmt.Sprintf("static const char *%s_parse(const char *json, %s *obj) {\n", funcName, s.StructName))
	}
	g.jsonOut.WriteString("    if (json == NULL || obj == NULL) return NULL;\n")
	g.jsonOut.WriteString("    memset(obj, 0, sizeof(*obj));\n")
	g.jsonOut.WriteString("    const char *p;\n")
	g.jsonOut.WriteString("    (void)p; // may be unused\n\n")

	for _, f := range s.Fields {
		g.jsonOut.WriteString(fmt.Sprintf("    p = find_key(json, \"%s\");\n", f.JSONName))
		g.jsonOut.WriteString("    if (p != NULL) {\n")

		if f.IsArray {
			g.jsonOut.WriteString("        size_t count = count_array(p);\n")
			g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = count;\n", f.CountField))
			g.jsonOut.WriteString("        if (count > 0) {\n")

			if f.IsRef {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(%s));\n", f.FieldName, f.RefType))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(%s));\n", f.FieldName, f.RefType))
				}
				refFuncName := g.funcName(f.RefSchemaName)
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("                %s_parse(alloc, elem, &obj->%s[i]);\n", refFuncName, f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("                %s_parse(elem, &obj->%s[i]);\n", refFuncName, f.FieldName))
				}
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsEnum {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(%s));\n", f.FieldName, f.EnumType))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(%s));\n", f.FieldName, f.EnumType))
				}
				enumFuncName := g.funcName(f.EnumSchemaName)
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				if g.useAllocator {
					g.jsonOut.WriteString("                char *str = parse_string(alloc, elem, &elem);\n")
					g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = %s_from_string(str);\n", f.FieldName, enumFuncName))
					g.jsonOut.WriteString("                json_free(alloc, str);\n")
				} else {
					g.jsonOut.WriteString("                char *str = parse_string(elem, &elem);\n")
					g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = %s_from_string(str);\n", f.FieldName, enumFuncName))
					g.jsonOut.WriteString("                free(str);\n")
				}
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsString {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(char*));\n", f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(char*));\n", f.FieldName))
				}
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = parse_string(alloc, elem, &elem);\n", f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = parse_string(elem, &elem);\n", f.FieldName))
				}
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsInt {
				ctype := "int64_t"
				if f.IsInt32 {
					ctype = "int32_t"
				}
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(%s));\n", f.FieldName, ctype))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(%s));\n", f.FieldName, ctype))
				}
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = (%s)parse_int(elem, &elem);\n", f.FieldName, ctype))
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsFloat {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(float));\n", f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(float));\n", f.FieldName))
				}
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = (float)parse_double(elem, &elem);\n", f.FieldName))
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsDouble {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(double));\n", f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(double));\n", f.FieldName))
				}
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = parse_double(elem, &elem);\n", f.FieldName))
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			} else if f.IsBool {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = json_calloc(alloc, count, sizeof(bool));\n", f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("            obj->%s = calloc(count, sizeof(bool));\n", f.FieldName))
				}
				g.jsonOut.WriteString("            const char *elem = array_first(p);\n")
				g.jsonOut.WriteString("            for (size_t i = 0; i < count && elem; i++) {\n")
				g.jsonOut.WriteString(fmt.Sprintf("                obj->%s[i] = parse_bool(elem, &elem);\n", f.FieldName))
				g.jsonOut.WriteString("                elem = array_next(elem);\n")
				g.jsonOut.WriteString("            }\n")
			}
			g.jsonOut.WriteString("        }\n")
		} else if f.IsEnum {
			enumFuncName := g.funcName(f.EnumSchemaName)
			if g.useAllocator {
				g.jsonOut.WriteString("        char *str = parse_string(alloc, p, &p);\n")
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = %s_from_string(str);\n", f.FieldName, enumFuncName))
				g.jsonOut.WriteString("        json_free(alloc, str);\n")
			} else {
				g.jsonOut.WriteString("        char *str = parse_string(p, &p);\n")
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = %s_from_string(str);\n", f.FieldName, enumFuncName))
				g.jsonOut.WriteString("        free(str);\n")
			}
		} else if f.IsRef {
			refFuncName := g.funcName(f.RefSchemaName)
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = json_calloc(alloc, 1, sizeof(%s));\n", f.FieldName, f.RefType))
				g.jsonOut.WriteString(fmt.Sprintf("        if (obj->%s) %s_parse(alloc, p, obj->%s);\n", f.FieldName, refFuncName, f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = calloc(1, sizeof(%s));\n", f.FieldName, f.RefType))
				g.jsonOut.WriteString(fmt.Sprintf("        if (obj->%s) %s_parse(p, obj->%s);\n", f.FieldName, refFuncName, f.FieldName))
			}
		} else if f.IsString {
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = parse_string(alloc, p, &p);\n", f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = parse_string(p, &p);\n", f.FieldName))
			}
		} else if f.IsInt {
			if f.IsInt32 {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = (int32_t)parse_int(p, &p);\n", f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = parse_int(p, &p);\n", f.FieldName))
			}
		} else if f.IsFloat {
			g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = (float)parse_double(p, &p);\n", f.FieldName))
		} else if f.IsDouble {
			g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = parse_double(p, &p);\n", f.FieldName))
		} else if f.IsBool {
			g.jsonOut.WriteString(fmt.Sprintf("        obj->%s = parse_bool(p, &p);\n", f.FieldName))
		}

		g.jsonOut.WriteString("    }\n\n")
	}

	g.jsonOut.WriteString("    return skip_value(json);\n")
	g.jsonOut.WriteString("}\n\n")

	// Public function
	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf("int %s_from_json(%s *alloc, const char *json, %s *obj) {\n", funcName, allocType, s.StructName))
		g.jsonOut.WriteString(fmt.Sprintf("    return %s_parse(alloc, json, obj) != NULL ? 0 : -1;\n", funcName))
	} else {
		g.jsonOut.WriteString(fmt.Sprintf("int %s_from_json(const char *json, %s *obj) {\n", funcName, s.StructName))
		g.jsonOut.WriteString(fmt.Sprintf("    return %s_parse(json, obj) != NULL ? 0 : -1;\n", funcName))
	}
	g.jsonOut.WriteString("}\n\n")
}

func (g *Generator) generateStructFree(s StructInfo) {
	funcName := g.funcName(s.Name)
	allocType := g.typeName("Allocator")

	if g.useAllocator {
		g.jsonOut.WriteString(fmt.Sprintf("void %s_free(%s *alloc, %s *obj) {\n", funcName, allocType, s.StructName))
	} else {
		g.jsonOut.WriteString(fmt.Sprintf("void %s_free(%s *obj) {\n", funcName, s.StructName))
	}
	g.jsonOut.WriteString("    if (obj == NULL) return;\n")

	for _, f := range s.Fields {
		if f.IsArray {
			if f.IsRef {
				refFuncName := g.funcName(f.RefSchemaName)
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("    for (size_t i = 0; i < obj->%s; i++) %s_free(alloc, &obj->%s[i]);\n", f.CountField, refFuncName, f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("    for (size_t i = 0; i < obj->%s; i++) %s_free(&obj->%s[i]);\n", f.CountField, refFuncName, f.FieldName))
				}
			} else if f.IsString {
				if g.useAllocator {
					g.jsonOut.WriteString(fmt.Sprintf("    for (size_t i = 0; i < obj->%s; i++) json_free(alloc, obj->%s[i]);\n", f.CountField, f.FieldName))
				} else {
					g.jsonOut.WriteString(fmt.Sprintf("    for (size_t i = 0; i < obj->%s; i++) free(obj->%s[i]);\n", f.CountField, f.FieldName))
				}
			}
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("    json_free(alloc, obj->%s);\n", f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("    free(obj->%s);\n", f.FieldName))
			}
		} else if f.IsRef {
			refFuncName := g.funcName(f.RefSchemaName)
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("    if (obj->%s) { %s_free(alloc, obj->%s); json_free(alloc, obj->%s); }\n", f.FieldName, refFuncName, f.FieldName, f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("    if (obj->%s) { %s_free(obj->%s); free(obj->%s); }\n", f.FieldName, refFuncName, f.FieldName, f.FieldName))
			}
		} else if f.IsString {
			if g.useAllocator {
				g.jsonOut.WriteString(fmt.Sprintf("    json_free(alloc, obj->%s);\n", f.FieldName))
			} else {
				g.jsonOut.WriteString(fmt.Sprintf("    free(obj->%s);\n", f.FieldName))
			}
		}
	}

	g.jsonOut.WriteString("}\n\n")
}

func (g *Generator) generateHandlers() {
	if g.doc.Paths == nil {
		return
	}

	pathList := make([]string, 0)
	for path := range g.doc.Paths.Map() {
		pathList = append(pathList, path)
	}
	sort.Strings(pathList)

	for _, path := range pathList {
		pathItem := g.doc.Paths.Value(path)
		if pathItem == nil {
			continue
		}

		operations := []struct {
			method string
			op     *openapi3.Operation
		}{
			{"get", pathItem.Get},
			{"post", pathItem.Post},
			{"put", pathItem.Put},
			{"delete", pathItem.Delete},
			{"patch", pathItem.Patch},
		}

		for _, item := range operations {
			if item.op == nil {
				continue
			}
			g.generateHandler(item.method, path, item.op)
		}
	}
	g.out.WriteString("\n")
}

func (g *Generator) generateHandler(method, path string, op *openapi3.Operation) {
	if op.Summary != "" {
		g.out.WriteString(fmt.Sprintf("// %s %s - %s\n", strings.ToUpper(method), path, op.Summary))
	} else {
		g.out.WriteString(fmt.Sprintf("// %s %s\n", strings.ToUpper(method), path))
	}

	funcName := g.operationToFuncName(method, path, op)
	g.out.WriteString(fmt.Sprintf("void %s(Req *req, Res *res);\n\n", funcName))
}

func (g *Generator) generateRoutes() {
	if g.doc.Paths == nil {
		return
	}

	routesFuncName := g.funcName("routes")
	g.out.WriteString(fmt.Sprintf("static inline void %s() {\n", routesFuncName))

	pathList := make([]string, 0)
	for path := range g.doc.Paths.Map() {
		pathList = append(pathList, path)
	}
	sort.Strings(pathList)

	for _, path := range pathList {
		pathItem := g.doc.Paths.Value(path)
		if pathItem == nil {
			continue
		}

		// Convert {param} to :param for ecewo
		ecePath := g.convertPathToEcewo(path)

		operations := []struct {
			method  string
			ecewoFn string
			op      *openapi3.Operation
		}{
			{"get", "get", pathItem.Get},
			{"post", "post", pathItem.Post},
			{"put", "put", pathItem.Put},
			{"delete", "del", pathItem.Delete},
			{"patch", "patch", pathItem.Patch},
		}

		for _, item := range operations {
			if item.op == nil {
				continue
			}
			funcName := g.operationToFuncName(item.method, path, item.op)
			g.out.WriteString(fmt.Sprintf("    %s(\"%s\", %s);", item.ecewoFn, ecePath, funcName))
			if item.op.Summary != "" {
				g.out.WriteString(fmt.Sprintf("    // %s\n", item.op.Summary))
			} else {
				g.out.WriteString("\n")
			}
		}
	}

	g.out.WriteString("}\n\n")
}

// convertPathToEcewo converts OpenAPI path params {id} to ecewo format :id
func (g *Generator) convertPathToEcewo(path string) string {
	result := path
	for {
		start := strings.Index(result, "{")
		if start == -1 {
			break
		}
		end := strings.Index(result, "}")
		if end == -1 {
			break
		}
		paramName := result[start+1 : end]
		result = result[:start] + ":" + paramName + result[end+1:]
	}
	return result
}

func (g *Generator) operationToFuncName(method, path string, op *openapi3.Operation) string {
	var name string

	if op.OperationID != "" {
		name = op.OperationID
	} else {
		pathPart := strings.ReplaceAll(path, "/", "_")
		pathPart = strings.ReplaceAll(pathPart, "{", "by_")
		pathPart = strings.ReplaceAll(pathPart, "}", "")
		pathPart = strings.Trim(pathPart, "_")
		name = method + "_" + pathPart
	}

	styled := g.applyStyle(name, g.funcStyle)
	if g.funcPrefix != "" {
		return g.funcPrefix + styled
	}
	return styled
}

// Helper functions

func (g *Generator) sortedSchemaNames() []string {
	names := make([]string, 0, len(g.doc.Components.Schemas))
	for name := range g.doc.Components.Schemas {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

func (g *Generator) sortedPropertyNames(props openapi3.Schemas) []string {
	names := make([]string, 0, len(props))
	for name := range props {
		names = append(names, name)
	}
	sort.Strings(names)
	return names
}

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

func wrapComment(s string) string {
	s = strings.ReplaceAll(s, "\n", " ")
	s = strings.ReplaceAll(s, "\r", "")
	return strings.TrimSpace(s)
}
