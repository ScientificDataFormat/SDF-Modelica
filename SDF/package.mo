within ;
package SDF "Scientific Data Format"
  extends .Modelica.Icons.Package;

  annotation (
    uses(Modelica(version="4.1.0")),
    version="0.4.6",
    conversion(
      noneFromVersion = "0.4.5",
      noneFromVersion = "0.4.4",
      noneFromVersion = "0.4.3",
      noneFromVersion = "0.4.2",
      noneFromVersion = "0.4.1",
      noneFromVersion = "0.4.0",
      noneFromVersion = "0.3.1",
      noneFromVersion = "0.3.0",
      noneFromVersion = "0.2.0"),
    Icon(
      coordinateSystem(extent={{-100,-100}, {100,100}}),
      graphics={
        Rectangle(
          extent={{-60,60}, {60,-60}},
          lineColor={95,95,95},
          fillColor={235,235,235},
          fillPattern=FillPattern.Solid),
        Line(
          points={{-20,60}, {-20,-60}},
          color={95,95,95}),
        Line(
          points={{20,60}, {20,-60}},
          color={95,95,95}),
        Line(
          points={{0,80}, {0,-40}},
          color={95,95,95},
          origin={20,20},
          rotation=90),
        Line(
          points={{0,80}, {0,-40}},
          color={95,95,95},
          origin={20,-20},
          rotation=90)}),
    Documentation(info="<html>
<p>The SDF package contains blocks and functions to read, write and interpolate multi-dimensional data using the <a href=\"https://github.com/ScientificDataFormat/SDF\">Scientific Data Format</a>.</p>
</html>"));
end SDF;
