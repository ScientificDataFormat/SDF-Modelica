within SDF.Internal.Functions;
impure function createGroup
  extends Modelica.Icons.Function;
  input String fileName;
  input String groupName;
  input String comment;
  output String errorMessage;
  external "C"  errorMessage = ModelicaSDF_create_group(fileName, groupName, comment) annotation (
  Library={"ModelicaSDF"},
  LibraryDirectory="modelica://SDF/Resources/Library");
end createGroup;
