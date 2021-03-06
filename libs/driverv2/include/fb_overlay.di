<interface name="fb_overlay">
   <!-- Setup the structure, but do not touch or enable the device -->

   <method rtype="uintptr_t" name="get_buffer">
   </method>

   <method rtype="void" name="set_buffer">
    <arg atype="uintptr_t" name="buff_phys" />
   </method>

   <method rtype="void" name="get_mode">
    <arg atype="uint32_t*" name="xres" />
    <arg atype="uint32_t*" name="yres" />
    <arg atype="uint32_t*" name="bpp" />
   </method>

   <method rtype="int" name="set_mode">
    <arg atype="uint32_t" name="xres" />
    <arg atype="uint32_t" name="yres" />
    <arg atype="uint32_t" name="bpp" />
   </method>

   <method rtype="int" name="set_pos">
    <arg atype="int" name="x" />
    <arg atype="int" name="y" />
   </method>
   <method rtype="int" name="get_pos">
    <arg atype="int*" name="x" />
    <arg atype="int*" name="y" />
   </method>

   <method rtype="int" name="set_size">
    <arg atype="int" name="width" />
    <arg atype="int" name="height" />
   </method>
   <method rtype="int" name="get_size">
    <arg atype="int*" name="width" />
    <arg atype="int*" name="height" />
   </method>

   <method rtype="int" name="enable" />
   <method rtype="int" name="disable" />




</interface>

