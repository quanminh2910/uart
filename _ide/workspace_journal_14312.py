# 2025-11-11T21:13:38.068876400
import vitis

client = vitis.create_client()
client.set_workspace(path="vitiswork")

platform = client.get_component(name="ARTY")
status = platform.build()

comp = client.get_component(name="app_component")
comp.build()

status = platform.build()

vitis.dispose()

