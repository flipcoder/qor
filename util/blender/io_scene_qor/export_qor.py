import json
import bpy
from mathutils import *

def mesh_triangulate(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()

def save(operator, context, filepath=""):

    buf = {}
    buf["nodes"] = []
    
    for base in bpy.context.scene.object_bases:
        
        node = {}
        obj = base.object
        
        if obj.type == "MESH":
            mesh = obj.to_mesh(bpy.context.scene, settings="PREVIEW", apply_modifiers=True)
            mesh_triangulate(mesh)
            node = {'type': 'mesh'}
        elif obj.type == "LAMP":
            node = {'type': 'light'}
        elif obj.type == "CAMERA":
            node = {'type': 'camera'}
        else:
            pass
        
        if node:
            buf["nodes"] += [node]
        
        out = open(filepath, "w")
        out.write(json.dumps(buf))
        out.close()

    return {"FINISHED"}

