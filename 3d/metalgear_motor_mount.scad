module motor() {
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
}

module motor_mount() {
    difference() {
        union() {
            translate([0, 0, (15 + 9 - 15) / 2 + 0.5])
                cube([14+1, 12+1, 10 + 15], center=true);
            translate([0, 6 - 0.5+0.5, 0])
                cube([24, 1, 15], center=true);
        }

        translate([0, 0, (15 + 9 - 15) / 2])
            cube([12+0.5, 10+0.5, 9 + 15 + 0.5], center=true);
        

        translate([0, 0, (15) / 2 + 9])
            for (t = [[4.5, 0, 0], [-4.5, 0, 0]]) {
                translate(t)
                    cylinder(d=1, h=5, center=true, $fn=16);
            }
        translate([0, 0, (15 + 10) / 2 + 9])
                cylinder(d=3+0.5, h=10, center=true, $fn=16);
            
        for (t = [[-9.5, 5, -4.5], [9.5, 5, -4.5], [-9.5, 5, 4.5], [9.5, 5, 4.5]]) {
            translate(t)
                rotate([90,0,0])
                    cylinder(d = 2, h = 10, center=true, $fn=16);
        }
            
    }
}

motor();

!intersection() {
    motor_mount();

    translate([0, -6 + 0.5-0.5, 9 / 2])
        cube([24, 3.5, 15+9], center=true);
}

difference() {
    motor_mount();

    translate([0, -6 + 0.5-0.5, 9 / 2])
        cube([24, 2, 15+9], center=true);
}