difference() {
    union() {
        translate([0, 0, (15 + 9) / 2])
        cube([12, 10, 9], center=true);
        intersection() {
            cube([12, 10, 15], center=true);
            cylinder(d=12, h=15, center=true, $fn=32);
        }

        translate([0, 0, (15 + 10) / 2 + 9])
            intersection() {
                translate([0, 0.25, 0])
                    cube([3, 2.5, 10], center=true);
                cylinder(d=3, h=10, center=true, $fn=16);
            }
    }

    translate([0, 0, (15) / 2 + 9])
        for (t = [[4.5, 0, 0], [-4.5, 0, 0]]) {
            translate(t)
                cylinder(d=1, h=5, center=true, $fn=16);
    }
}