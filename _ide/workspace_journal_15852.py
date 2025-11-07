# 2025-11-06T02:19:20.067941700
import vitis

client = vitis.create_client()
client.set_workspace(path="vitiswork")

advanced_options = client.create_advanced_options_dict(dt_overlay="0")

platform = client.create_platform_component(name = "ARTY",hw_design = "$COMPONENT_LOCATION/../../Do_an_v1/artyz7_20_platform.xsa",os = "standalone",cpu = "ps7_cortexa9_0",domain_name = "standalone_ps7_cortexa9_0",generate_dtb = False,advanced_options = advanced_options,compiler = "gcc")

platform = client.get_component(name="ARTY")
status = platform.build()

vitis.dispose()

