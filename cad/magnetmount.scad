$fn=128;

fw = 48.3;   // fan width

h  = 4;      // base height
r  = 5;      // edge width

pn = 7;      // number of pins
ph = 7;      // pin height
pa = 20;     // pin angle

mw = 10.25;  // magnet width
ml = 20.25;  // magnet length
mb = 17;     // magnet bar width
g  = 10;     // magnet gap

d=fw+r+r;    // total diameter
cl=d-r;      // magnet bar length
a=360/pn;    // segment angle

difference() {
  union() {
    difference() {
      translate([0,0,0]) cylinder(h+ph,d/2,d/2);
      translate([0,0,-1]) cylinder(h+ph+2,d/2-r,d/2-r);
      for (i =[0:1:pn-1]) {
        pin(a*i);
      }
    }
    translate([-cl/2,-mb/2,0]) cube([cl,mb,h]);
    rotate ([0,0,90]) translate([-cl/2,-(mb-mw)/4-d/7,0]) cube([cl,(mb-mw)/2,h]);
    rotate ([0,0,90]) translate([-cl/2,-(mb-mw)/4+d/7,0]) cube([cl,(mb-mw)/2,h]);
  }
  translate([-ml-g/2,-mw/2,-1]) cube([ml,mw,h]);
  translate([g/2,-mw/2,-1]) cube([ml,mw,h]);
}

module pin(r) {
  translate([0,0,0.01]) rotate([0,0,r]) hull() {
    translate([0,0,h]) cylinder(h+ph,0.1,0.1);
    translate([-d,0,h]) cylinder(h+ph,0.1,0.1);
    rotate ([0,0,a-pa])union() {
      translate([0,0,h]) cylinder(h+ph,0.1,0.1);
      translate([-d,0,h]) cylinder(h+ph,0.1,0.1);
    }
  }
}
