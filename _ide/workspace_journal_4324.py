# 2025-11-08T00:59:37.884368800
import vitis

client = vitis.create_client()
client.set_workspace(path="vitiswork")

comp = client.create_app_component(name="app_component",platform = "$COMPONENT_LOCATION/../ARTY/export/ARTY/ARTY.xpfm",domain = "standalone_ps7_cortexa9_0")

vitis.dispose()

