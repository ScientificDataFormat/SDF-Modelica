within SDF.Internal.Functions;
impure function makeDatasetDouble2D
  extends Modelica.Icons.Function;
  input String fileName;
  input String datasetName;
  input Real values[:,:];
  input String comment;
  input String displayName;
  input String unit;
  input String displayUnit;
  input Boolean relativeQuantity;
  output String errorMessage;
protected
  Integer dims[:] = { size(values, 1), size(values, 2)};
external "C" errorMessage = ModelicaSDF_make_dataset_double(
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
end makeDatasetDouble2D;
