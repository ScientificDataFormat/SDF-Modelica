within SDF;
model NDTable "N-dimensional lookup-table"
extends Modelica.Blocks.Interfaces.MISO;

parameter Boolean readFromFile = true "Read data from file" annotation(Evaluate=true);
parameter String filename = "" "File name" annotation (Dialog(loadSelector(filter="SDF Files (*.sdf);;All Files (*.*)", caption="Select SDF file")));
parameter String dataset = "" "Dataset name";
parameter String dataUnit = "" "Data unit";
parameter String scaleUnits[nin] = fill("", nin) "Scale units";

parameter SDF.Types.InterpolationMethod interpMethod=SDF.Types.InterpolationMethod.Linear
    "Interpolation method";
parameter SDF.Types.ExtrapolationMethod extrapMethod=SDF.Types.ExtrapolationMethod.None
    "Extrapolation method";
parameter Boolean enableDy_du=false "Enable partial derivative output dy_du"
    annotation(Evaluate=true);

parameter Real data[:] = { 0}   "Table data (as returned by readTableData())" annotation(Dialog(enable=not readFromFile), Evaluate=true);

Modelica.Blocks.Interfaces.RealOutput dy_du[nin] if enableDy_du
    "Partial derivatives of y with respect to each input u[i]"
    annotation (Placement(
      transformation(extent={{-10,-10},{10,10}},
        rotation=0,
        origin={110,-60}),
      visible=enableDy_du));

protected
  Modelica.Blocks.Interfaces.RealInput local_dy_du[nin] annotation(HideResult=true);

  function evaluate
    input SDF.Types.ExternalNDTable table;
    input Real[:] params;
    input SDF.Types.InterpolationMethod interpMethod;
    input SDF.Types.ExtrapolationMethod extrapMethod;
    output Real value;
    external "C" value = ModelicaNDTable_evaluate(table, size(params, 1), params, interpMethod, extrapMethod) annotation (
      Include="#include <ModelicaNDTable.c>",
      IncludeDirectory="modelica://SDF/Resources/C-Sources");
    annotation(
      derivative(zeroDerivative = table,
               zeroDerivative = interpMethod,
               zeroDerivative = extrapMethod) = evaluate_der);
  end evaluate;

  function evaluate_der
    input SDF.Types.ExternalNDTable table;
    input Real[:] params;
    input SDF.Types.InterpolationMethod interpMethod;
    input SDF.Types.ExtrapolationMethod extrapMethod;
    input Real[size(params, 1)] der_params;
    output Real der_value;
    external "C" der_value = ModelicaNDTable_evaluate_derivative(
      table,
      size(params, 1),
      params,
      interpMethod,
      extrapMethod,
      der_params)
    annotation (
      Include="#include <ModelicaNDTable.c>",
      IncludeDirectory="modelica://SDF/Resources/C-Sources");
  end evaluate_der;

  function evaluatePartialDerivatives
    input SDF.Types.ExternalNDTable table;
    input Real[:] params;
    input SDF.Types.InterpolationMethod interpMethod;
    input SDF.Types.ExtrapolationMethod extrapMethod;
    output Real derivatives[size(params, 1)];
    external "C" ModelicaNDTable_evaluate_partial_derivatives(
      table,
      size(params, 1),
      params,
      interpMethod,
      extrapMethod,
      derivatives) annotation (
      Include="#include <ModelicaNDTable.c>",
      IncludeDirectory="modelica://SDF/Resources/C-Sources");
  end evaluatePartialDerivatives;

  SDF.Types.ExternalNDTable externalTable=SDF.Types.ExternalNDTable(nin, if readFromFile then SDF.Functions.readTableData(
        Modelica.Utilities.Files.loadResource(filename),
        dataset,
        dataUnit,
        scaleUnits) else data);

equation
  y = evaluate(
    externalTable,
    u,
    interpMethod,
    extrapMethod);

  if enableDy_du then
    connect(dy_du, local_dy_du);
    local_dy_du = evaluatePartialDerivatives(
      externalTable,
      u,
      interpMethod,
      extrapMethod);
  else
    local_dy_du = zeros(nin);
  end if;


  annotation (Documentation(info="<html>
<body>
<p>The <strong>NDTable</strong> block is a multi-dimensional lookup-table (up to 32 dimensions) that supports various inter- and extrapolation methods.</p>
</body>
</html>"), Icon(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},
          {100,100}}), graphics={
      Rectangle(
          extent={{-58,60},{62,-60}},
          lineColor={47,49,172},
          fillColor={255,255,125},
          fillPattern=FillPattern.Solid),
      Line(
        points={{-18,60},{-18,-60}},
        color={161,159,189}),
      Line(
        points={{22,60},{22,-60}},
        color={161,159,189}),
      Line(
        points={{1,64},{1,-56}},
        color={161,159,189},
          origin={6,-21},
          rotation=90),
      Line(
        points={{1,76},{1,-44}},
        color={161,159,189},
          origin={18,19},
          rotation=90),
        Text(
          extent={{-147,-152},{153,-112}},
          lineColor={0,0,0},
          textString="nin=%nin"),
      Rectangle(
          extent={{-58,60},{62,-60}},
          lineColor={47,49,172})}));
end NDTable;
