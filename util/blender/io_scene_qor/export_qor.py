import os
import sys
import json
import bpy
import itertools
from mathutils import *
from bpy_extras.io_utils import axis_conversion

def mesh_triangulate(mesh):
    import bmesh
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.triangulate(bm, faces=bm.faces)
    bm.to_mesh(mesh)
    bm.free()

blender_matrix = axis_conversion(
    from_forward='Y',
    from_up='Z',
    to_forward='-Z',
    to_up='Y'
).to_4x4()

# def from_blender_space(m):
#     r = m * blender_matrix
#     r[2][0], r[0][0] = r[0][0], r[2][0]
#     return r

def basename(p):
    f = p.rfind('\\')
    if f == -1:
        f = p.rfind('/')
    if f == -1:
        return p
    return p[f+1:]

def vec(v):
    #v.z, v.x, v.y = -v.x, v.y, v.z
    #return v * blender_matrix
    return v

def invert_uv(uv):
    uv[1] = 1.0 - uv[1]
    return uv

def mat(m):
    #v.z, v.x, v.y = -v.x, v.y, v.z
    m = m.copy()
    m = blender_matrix * m
    a = list(itertools.chain(*map(lambda v: v.to_tuple(), m)))
    r = [a[0],a[4],a[8],a[12],
         a[1],a[5],a[9],a[13],
         a[2],a[6],a[10],a[14],
         a[3],a[7],a[11],a[15]]
    return r

def iterate_node(scene, obj, context, nodes):
    node = {}
    
    if obj.type == "MESH":
        # mesh = obj.to_mesh(bpy.context.scene, settings="PREVIEW", apply_modifiers=True)
        # mesh_triangulate(mesh)
        
        node = {
            'name': obj.name,
            'data': obj.data.name,
            'type': 'mesh',
            'matrix': mat(obj.matrix_local)
        }
    elif obj.type == "SPEAKER":
        node = {
            'name': obj.name,
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
        node["nodes"] = {}
    
    for ch in obj.children:
        iterate_node(scene, ch, context, node["nodes"])
    
    if node:
        nodes[node["name"]] = node

def iterate_data(scene, obj, context, entries):
    doc = {}
    
    if hasattr(obj, 'type'):
        dtype = obj.type
    else:
        dtype = obj.data.type
    
    if dtype == "MESH":
        # mesh = obj.data
        mesh = obj.to_mesh(scene, True, 'PREVIEW', calc_tessface=True)
        mesh_triangulate(mesh)
        mesh.update(calc_tessface=True)
        vertices = []
        normals = []
        # indices = []
        wrap = []
        colors = []
        idx = 0
        mats = []
        images = []

        # for face in mesh.tessfaces:
        #     mat += [mesh.uv_textures[0].data[face.index].image.name]

        # for m in mat:
        for face in mesh.tessfaces:
            assert len(face.vertices) == 3
            verts = face.vertices[:]
            # if mesh.uv_textures[0].data[face.index].image:
                # images += [mesh.uv_textures[0].data[face.index].image.name]
            if len(obj.data.materials)>0 and hasattr(obj.data.materials[face.material_index].active_texture,"image"):
                images += [basename(obj.data.materials[face.material_index].active_texture.image.filepath)]
            else:
                images += [""] # no image
            for v in verts:
                vertices += list(mesh.vertices[v].co.to_tuple())
                normals += list(mesh.vertices[v].normal.to_tuple())
                # indices += [v]
                # idx += 1
        # for v in mesh.vertices:
        #     vertices += list(v.co.to_tuple())
        #     normals += list(v.normal.to_tuple())
        if mesh.tessface_uv_textures:
            for e in mesh.tessface_uv_textures.active.data:
                wrap += invert_uv(list(e.uv1.to_tuple()))
                wrap += invert_uv(list(e.uv2.to_tuple()))
                wrap += invert_uv(list(e.uv3.to_tuple()))
        if mesh.tessface_vertex_colors:
            for e in mesh.tessface_vertex_colors.active.data:
                colors += list(e.color1.to_tuple())
                colors += list(e.color2.to_tuple())
                colors += list(e.color3.to_tuple())
        
        img = None
        try:
            img = mesh.uv_textures[0].data[0].image.name
        except:
            pass
        
        doc = {
            'name': obj.data.name,
            'image': img,
            'images': images,
            'type': 'mesh',
            'vertices': vertices,
            'normals': normals,
            # 'indices': indices,
            'wrap': wrap,
            'colors': colors
        }

        # TODO split doc data based on assigned images
        docs = {}
        while vertices:
            if not images[0] in docs:
                docs[images[0]] = {}
            # if not 'vertices' in docs[images[0]]:
                docs[images[0]]['vertices'] = []
            # if not 'normals' in docs[images[0]]:
                docs[images[0]]['normals'] = []
            # if not 'wrap' in docs[images[0]]:
                docs[images[0]]['wrap'] = []
            # if not 'colors' in docs[images[0]]:
                docs[images[0]]['colors'] = []

            docs[images[0]]['vertices'] += vertices[0:9]
            vertices = vertices[9:]
            if normals:
                docs[images[0]]['normals'] += normals[0:9]
                normals = normals[9:]
            if wrap:
                docs[images[0]]['wrap'] += wrap[0:6]
                wrap = wrap[6:]
            if colors:
                docs[images[0]]['colors'] += colors[0:9]
                colors = colors[9:]
            images = images[1:]

        for k,v in docs.items():
            name = obj.data.name + ":" + k
            v["name"] = name
            v["image"] = k
            entries[name] = v

        return

    elif dtype == "SURFACE":
        name = basename(obj.name)
        doc = {
            'name': name,
            'type': 'material'
        }
        if obj.active_texture:
            doc['texture'] = basename(obj.active_texture.name)
            try:
                doc['image'] = basename(obj.active_texture['image'].image.name) #.filepath
            except KeyError:
                # print "No image for texture"
                pass
    elif dtype == "TEXTURE":
        name = basename(obj.name)
        doc = {
            'name': name,
            'type': 'material'
        }
        if 'image' in obj:
            doc['image'] = basename(obj.image.filepath)
    elif dtype == "IMAGE":
        name = basename(obj.name)
        if hasattr('obj', 'filepath'):
            filepath = basename(obj.filepath)
            doc = {
                'name': name,
                'type': 'material', # create standalone texture from image
                'image': filepath
            }
        else:
            return # bad
    else:
        pass
    
    if doc:
        entries[doc["name"]] = doc

def save(operator, context, filepath=""):

    buf = {}
    
    buf['type'] = 'scene'
    buf["gravity"] = vec(bpy.context.scene.gravity).to_tuple(4)
    buf['properties'] = {}
    
    buf["nodes"] = {}
    for base in bpy.context.scene.object_bases:
        iterate_node(bpy.context.scene, base.object, context, buf["nodes"])
    buf["data"] = {}
    for obj in itertools.chain(bpy.data.objects, bpy.data.materials, bpy.data.textures, bpy.data.images):
        #if obj.type not in buf["data"]:
        #    buf["data"][obj.type] = []
        name = None
        try:
            name = obj.data.name
        except AttributeError:
            name = obj.name
        if name in buf["data"]:
            continue # already inserted (instanced)
        iterate_data(bpy.context.scene, obj, context, buf["data"])
    #for obj in bpy.data.materials:
    #    if obj.type not in buf["data"]:
    #        buf["data"][obj.type] = []
    #    elif data.name in buf["data"][obj.type]:
    #        continue # already inserted (instanced)
    #    iterate_data(bpy.context.scene, obj, context, buf["data"][obj.type])
    #for obj in bpy.data.textures:
    #    if obj.type not in buf["data"]:
    #        buf["data"][obj.type] = []
    #    elif data.name in buf["data"][obj.type]:
    #        continue # already inserted (instanced)
    #    iterate_data(bpy.context.scene, obj, context, buf["data"][obj.type])
    
    out = open(filepath, "w")
    out.write(json.dumps(buf,indent=4,sort_keys=True))
    out.close()

    return {"FINISHED"}

