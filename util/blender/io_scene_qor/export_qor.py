import json
import bpy
import itertools
from mathutils import *

def mesh_triangulate(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()

blender_matrix = Matrix((
    (0.0, 1.0, 0.0),
    (0.0, 0.0, 1.0),
    (-1.0, 0.0, 0.0)
))

#def from_blender_space(m):
#    r = m * blender_matrix
    #r[2][0], r[0][0] = r[0][0], r[2][0]
    #return r

def vec(v):
    v.z, v.x, v.y = -v.x, v.y, v.z
    #return v * blender_matrix
    return v

def mat(m):
    #v.z, v.x, v.y = -v.x, v.y, v.z
    return m * blender_matrix

def iterate_object(scene, obj, context, buf):
    node = {}
    
    if obj.type == "MESH":
        mesh = obj.to_mesh(bpy.context.scene, settings="PREVIEW", apply_modifiers=True)
        mesh_triangulate(mesh)
        node = {
            'name': obj.name,
            'type': 'mesh',
            'matrix': list(itertools.chain(*map(lambda v: v.to_tuple(), (obj.matrix_local))))
        }
    elif obj.type == "LAMP":
        node = {
            'name': obj.name,
            'type': 'light',
            'matrix': list(itertools.chain(*map(lambda v: v.to_tuple(), (obj.matrix_local))))
        }
    elif obj.type == "CAMERA":
        node = {
            'name': obj.name,
            'type': 'camera',
            'matrix': list(itertools.chain(*map(lambda v: v.to_tuple(), (obj.matrix_local))))
        }
    else:
        pass
    
    for ch in obj.children:
        iterate_object(scene, ch, context, node)
    
    if node:
        buf["nodes"] += [node]

def save(operator, context, filepath=""):

    buf = {}
    buf["nodes"] = []
    
    buf["scene"] = {}
    buf["scene"]["gravity"] = vec(bpy.context.scene.gravity).to_tuple(4)
    
    for base in bpy.context.scene.object_bases:
        iterate_object(bpy.context.scene, base.object, context, buf)
        
    out = open(filepath, "w")
    out.write(json.dumps(buf, indent=4))
    out.close()

    return {"FINISHED"}

