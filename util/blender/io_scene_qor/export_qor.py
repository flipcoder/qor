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

#blender_matrix = Matrix((
#    (0.0, 1.0, 0.0),
#    (0.0, 0.0, 1.0),
#    (-1.0, 0.0, 0.0)
#))

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
    return list(itertools.chain(*map(lambda v: v.to_tuple(), m)))
    #return m * blender_matrix

def iterate_node(scene, obj, context, nodes):
    node = {}
    
    if obj.type == "MESH":
        mesh = obj.to_mesh(bpy.context.scene, settings="PREVIEW", apply_modifiers=True)
        #mesh_triangulate(mesh)
                
        node = {
            'name': obj.name,
            'data': obj.data.name,
            'type': 'mesh',
            'matrix': mat(obj.matrix_local)
        }
    elif obj.type == "SPEAKER":
        node = {
            'name': obj.name,
            'data': obj.data.name,
            'type': 'sound',
            'volume': obj.data.volume,
            'pitch': obj.data.pitch,
            'matrix': mat(obj.matrix_local)
        }
        if obj.data.sound:
            node["sound"] = obj.data.sound.name
    elif obj.type == "LAMP":
        node = {
            'name': obj.name,
            'data': obj.data.name,
            'type': 'light',
            'light': obj.data.type.lower(),
            'energy': obj.data.energy,
            'color': list(obj.data.color),
            'specular': obj.data.use_specular,
            'diffuse': obj.data.use_diffuse,
            'distance': obj.data.distance,
            'matrix': mat(obj.matrix_local)
        }
    elif obj.type == "CAMERA":
        node = {
            'name': obj.name,
            'data': obj.data.name,
            'type': 'camera',
            'matrix': mat(obj.matrix_local)
        }
    elif obj.type == "EMPTY":
        node = {
            'name': obj.name,
            'type': 'node',
            'matrix': mat(obj.matrix_local)
        }
    else:
        pass
    
    if node:
        if obj.keys():
            node["properties"] = {}
        for k in obj.keys():
            if not k.startswith('_'):
                if hasattr(obj.get(k), '__dict__'):
                    node["properties"][k] = obj.get(k)
                
    if obj.children:
        node["nodes"] = []
    
    for ch in obj.children:
        iterate_node(scene, ch, context, node["nodes"])
    
    if node:
        nodes += [node]

def iterate_data(scene, obj, context, entries):
    doc = {}
    
    if hasattr(obj, 'type'):
        dtype = obj.type
    else:
        dtype = obj.data.type
    
    if dtype == "MESH":
        mesh = obj.to_mesh(scene, True, 'PREVIEW')
        #obj.data.update(calc_tessface=True)
        vertices = []
        normals = []
        indices = []
        wrap = []
        colors = []
        idx = 0
        for face in obj.data.polygons:
            verts = face.vertices[:]
            for v in verts:
                vertices += list(obj.data.vertices[v].co.to_tuple())
                normals += list(obj.data.vertices[v].normal.to_tuple())
                indices += [idx]
                ++idx
        if mesh.tessface_uv_textures:
            for e in mesh.tessface_uv_textures.active.data:
                wrap += list(e.uv1.to_tuple())
                wrap += list(e.uv2.to_tuple())
                wrap += list(e.uv3.to_tuple())
        if mesh.tessface_vertex_colors:
            for e in mesh.tessface_vertex_colors.active.data:
                colors += list(e.color1.to_tuple())
                colors += list(e.color2.to_tuple())
                colors += list(e.color3.to_tuple())
        doc = {
            'name': obj.data.name,
            'type': 'mesh',
            'vertices': vertices,
            'indices': indices,
            'wrap': wrap,
            'colors': colors
        }
    elif dtype == "SURFACE": # material
        doc = {
            'name': obj.name,
            'type': 'material',
            'texture': obj.active_texture.name,
        }
    elif dtype == "TEXTURE":
        doc = {
            'name': obj.name,
            'type': 'texture',
            'image': obj.image
        }
        if 'image' in obj:
            doc['image'] = obj.image
    else:
        pass
    
    if doc:
        entries += [doc]

def save(operator, context, filepath=""):

    buf = {}
    
    buf["scene"] = {}
    buf["scene"]["gravity"] = vec(bpy.context.scene.gravity).to_tuple(4)
    buf["scene"]['properties'] = {}
    
    buf["scene"]["nodes"] = []
    for base in bpy.context.scene.object_bases:
        iterate_node(bpy.context.scene, base.object, context, buf["scene"]["nodes"])
    buf["scene"]["data"] = []
    for obj in itertools.chain(bpy.data.objects, bpy.data.materials, bpy.data.textures):
        #if obj.type not in buf["scene"]["data"]:
        #    buf["scene"]["data"][obj.type] = []
        name = None
        try:
            name = obj.data.name
        except AttributeError:
            name = obj.name
        if name in buf["scene"]["data"]:
            continue # already inserted (instanced)
        iterate_data(bpy.context.scene, obj, context, buf["scene"]["data"])
    #for obj in bpy.data.materials:
    #    if obj.type not in buf["scene"]["data"]:
    #        buf["scene"]["data"][obj.type] = []
    #    elif data.name in buf["scene"]["data"][obj.type]:
    #        continue # already inserted (instanced)
    #    iterate_data(bpy.context.scene, obj, context, buf["scene"]["data"][obj.type])
    #for obj in bpy.data.textures:
    #    if obj.type not in buf["scene"]["data"]:
    #        buf["scene"]["data"][obj.type] = []
    #    elif data.name in buf["scene"]["data"][obj.type]:
    #        continue # already inserted (instanced)
    #    iterate_data(bpy.context.scene, obj, context, buf["scene"]["data"][obj.type])
    
    out = open(filepath, "w")
    out.write(json.dumps(buf, indent=4))
    out.close()

    return {"FINISHED"}

