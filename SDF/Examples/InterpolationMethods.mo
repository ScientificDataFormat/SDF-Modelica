within SDF.Examples;
model InterpolationMethods "Interpolate a sampled signal using different interpolation and extrapolation methods"
  extends Modelica.Icons.Example;

  parameter Real x[:] = linspace(0, 2, 6) * Modelica.Constants.pi;
  parameter Real y[size(x, 1)] = Modelica.Math.sin(x);
  parameter Real data[:] = cat(1, {1}, {size(x, 1)}, x, y);

  parameter String filename = Modelica.Utilities.Files.loadResource("modelica://SDF/Resources/Data/Examples/sine.sdf");
  parameter String dataset = "/y";

  Modelica.Blocks.Sources.Ramp ramp(
    duration=1,
    offset=-1,
    height=8.3)
    annotation (Placement(transformation(extent={{-80,-30},{-60,-10}})));

  NDTable hold(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Hold,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{-40,40},{-20,60}})));

  NDTable nearest(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Nearest,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{0,40},{20,60}})));

  NDTable linear(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Linear,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{40,40},{60,60}})));

  NDTable akima(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Akima,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{-40,0},{-20,20}})));

  NDTable fritsch_butland(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.FritschButland,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{0,0},{20,20}})));

  NDTable steffen(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Steffen,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{40,0},{60,20}})));

  NDTable extrap_hold(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Linear,
    extrapMethod=SDF.Types.ExtrapolationMethod.Hold,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{-20,-60},{0,-40}})));

  NDTable extrap_linear(
    nin=1,
    readFromFile=false,
    data=data,
    interpMethod=SDF.Types.InterpolationMethod.Linear,
    extrapMethod=SDF.Types.ExtrapolationMethod.Linear,
    filename=filename,
    dataset=dataset)
    annotation (Placement(transformation(extent={{20,-60},{40,-40}})));

equation
  connect(ramp.y, hold.u[1]) annotation (Line(points={{-59,-20},{-50,-20},{-50,
          50},{-42,50}},
                     color={0,0,127}));
  connect(akima.u[1], ramp.y) annotation (Line(points={{-42,10},{-50,10},{-50,
          -20},{-59,-20}},
                    color={0,0,127}));
  connect(nearest.u[1], ramp.y) annotation (Line(points={{-2,50},{-10,50},{-10,
          -20},{-59,-20}},
                       color={0,0,127}));
  connect(fritsch_butland.u[1], ramp.y) annotation (Line(points={{-2,10},{-10,
          10},{-10,-20},{-59,-20}},
                                color={0,0,127}));
  connect(linear.u[1], ramp.y) annotation (Line(points={{38,50},{30,50},{30,-20},
          {-59,-20}},
                    color={0,0,127}));
  connect(steffen.u[1], ramp.y) annotation (Line(points={{38,10},{30,10},{30,
          -20},{-59,-20}},
                    color={0,0,127}));
  connect(extrap_hold.u[1], ramp.y) annotation (Line(points={{-22,-50},{-30,-50},
          {-30,-20},{-59,-20}},
                            color={0,0,127}));
  connect(extrap_linear.u[1], ramp.y) annotation (Line(points={{18,-50},{10,-50},
          {10,-20},{-59,-20}},
                           color={0,0,127}));
  annotation (
    Diagram(coordinateSystem(preserveAspectRatio=false, extent={{-100,-100},{
            100,100}})),
    Icon(coordinateSystem(extent={{-100,-100},{100,120}})));
end InterpolationMethods;
