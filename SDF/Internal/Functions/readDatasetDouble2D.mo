within SDF.Internal.Functions;
impure function readDatasetDouble2D
  extends Modelica.Icons.Function;
  input String fileName;
  input String datasetName;
  input String unit;
  output String errorMessage;
  output Real data[getDatasetDims(fileName, datasetName)*{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, getDatasetDims(fileName, datasetName)*{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}];
  external "C" errorMessage= ModelicaSDF_read_dataset_double(fileName, datasetName, unit, data) annotation (
  Library={"ModelicaSDF"},
  LibraryDirectory="modelica://SDF/Resources/Library");
  annotation(__Dymola_impureConstant=true);
end readDatasetDouble2D;
