Mechanical Design Journal
=========================

The decision to make the submarine resemble a Los Angeles class SSN is because this class has a simplistic look. Making a realistic scale model is not priority.

The decision to use twin propellers without rudders is for simplicity. Plus, rudders only work while in forward motion.

The decision to use a WTC (water tight cylinder) as the housing is because it is easy to seal with just end-caps and o-rings. No adhesives or custom shaped gaskets are required. The tube should be polycarbonate for strength, it is transparent, and more importantly, less prone to cracking than acrylic.

The diameter of the WTC tube is chosen to be 2.5 inches. This allows two 36mm propellers, and their driving motors, to fit side by side. The parts to implement this drive system is easily purchased from Amazon. The WTC tube can be purchased from McMaster-Carr.

With the WTC tube diameter chosen, the length is determined by the shape of the Los Angeles class SSN. The full length of the submarine should be about 600mm long, and the WTC tube should be about 450mm long.

There is two threaded rods running along the inside of the WTC tube. Internal components of the submarine, such as the pump, battery, and ballast, are placed along these rods with the help of 3D printed parts. This way allows for easy rearrangement and movement of these parts, which allows for the submarine center-of-gravity to be shifted easily. Ideally, the submarine is level when it is surfaced, and when it is underwater, the ballast bladder is at the center-of-gravity.

The drive motors should be 280 sized DC brushed motors, but the design can be easily adjusted to use 130 sized motors as well.

The outer hull of the submarine is 3D printed in 6 sections that magnetically attach to each other. The bow and stern are also 3D printed, and are attached to the WTC using screws. The hull pieces do not need to be waterproof.

## Diving

The diving action is done with a ballast system consisting of a bladder made of a balloon, and a peristaltic pump. The peristaltic pump allows the system to pump bidirectionally without the usage of any valves (even if it leaks, it will leak water outwards, which is still safe). The downside is that there's no safety, no redundancy, and the entire WTC must be airtight. The usage of a balloon bladder, instead of a water chamber, is to make it easy to shift the position of the ballast, and it requires no additional sealing against the WTC walls. As I am not very experienced, I am avoiding the use of valves, compressed air, and pistons.

If the bladder takes up 33% of the WTC's volume, then the air pressure will rise by 0.5 atm, roughly 7.3 PSI. The front and rear end-caps have a surface area of 5 sq-in. This means each end-cap will be under 37 lbs of force. Each end-cap will be secured to the WTC with three M3 screws radially. It is highly unlikely that the bladder will be filled to that extent.

Assume the whole submarine is a 600mm long and 80mm diameter cylinder. The volume is 3.016L, equivalent to 3016 grams of water at neutral buoyancy, which is the maximum weight of the submarine allowed while the ballast is empty. The WTC has a volume of 1.42L and 33% of that is about 475 additional grams of water possible. Roughly, the submarine should weigh less than 2900 grams (I made a very rough guess on safety margin) when finished. Additional mass can be added to achieve perfect neutral buoyancy after.

Water is an incompressible fluid, its density does not change by any appreciable amount for small depths like a swimming pool. I cannot determine the maximum dive depth of the submarine. What will happen is that the buoyancy force will always be acting on it, it will almost always be rising or sinking by very small amounts, sometimes so small that it appears to be neutrally buoyant. Originally I wanted to design a limit to the ballast bladder so it can't sink below a particular depth, but this is practically impossible. Instead, I will only be able to limit its rate of decent.

With a pump capable of moving roughly 1mL/s of water, it seems like if I wanted to dive or rise 1 meter vertically, it would take under 8 seconds of pumping. At neutral buoyancy, 1 second of additional pumping only results in a 0.08m/s change in velocity after 1 meter, and take roughly 24 seconds to rise or dive another 1 meter. Pumping for 5 seconds will get you enough vertical velocity to move 1 meter in just 10 seconds. A full minute of pumping is only 60 additional mL of water and it makes the vertical velocity fast enough to move 1 meter in 3 seconds.

This feels like enough control for human control without computerized assistance. Sensing depth is incredibly difficult at such small scale. The RSSI and LQI of the radios might be an easier way of estimating depth.

The bladder will never need to occupy that much volume, and the projected forces on the front and rear end-caps is incredibly overestimated.
