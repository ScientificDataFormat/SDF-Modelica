within SDF.Internal.Functions;
impure function makeDatasetInteger2D
  extends Modelica.Icons.Function;
  input String fileName;
  input String datasetName;
  input Integer values[:,:];
  input String comment;
  input String displayName;
  input String unit;
  input String displayUnit;
  input Boolean relativeQuantity;
  output String errorMessage;
protected
  Integer dims[:] = { size(values, 1), size(values, 2)};
external "C" errorMessage = ModelicaSDF_make_dataset_int(
         fileName,
         datasetName,
         2,
         dims,
         values,
         comment,
         displayName,
         unit,
         displayUnit,
         relativeQuantity) annotation (
  Library={"ModelicaSDF"},
  LibraryDirectory="modelica://SDF/Resources/Library");
end makeDatasetInteger2D;
