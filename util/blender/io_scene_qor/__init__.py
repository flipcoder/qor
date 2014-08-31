bl_info = {
    "name":         "Qor JSON Scene Export",
    "author":       "Grady O'Connell",
    "blender":      (2,6,8),
    "version":      (0,0,1),
    "location":     "File > Import-Export",
    "description":  "Import-Export Qor JSON data format (export only)",
    "category":     "Import-Export",
    "wiki_url":     "https://github.com/qor",
    "tracker_url":  "https://github.com/qor",
}

import bpy
from bpy.props import *
from bpy_extras.io_utils import ExportHelper, ImportHelper

class ExportQor(bpy.types.Operator, ExportHelper):
    bl_idname = "export.json"
    bl_label = "Export Qor JSON"

    filename_ext = ".json"
    
    def invoke(self, context, event):
        return ExportHelper.invoke(self, context, event)
    
    @classmethod
    def poll(cls, context):
        return context.active_object != None
    
    def execute(self, context):
        if not self.properties.filepath:
            raise Exception("filename not set")
        
        filepath = self.filepath
        
        import export_qor
        return export_qor.save(self, context, **self.properties)

def menu_func_export(self, context):
    default_path = bpy.data.filepath.replace(".blend", ".json")
    self.layout.operator(ExportQor.bl_idname, text="Qor JSON (.json)").filepath = default_path

#def menu_func_import(self, context):
    #self.layout.operator(ImportQor.bl_idname, text="Qor JSON (.json)")

def register():
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(menu_func_export)
    #bpy.types.INFO_MT_file_import.append(menu_func_import)

def unregister():
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)
    #bpy.types.INFO_MT_file_import.remove(menu_func_import)

if __name__ == "__main__":
    register()

