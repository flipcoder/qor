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

prec = 5

def rounded(l, prec):
    r = []
    for e in l:
        r += [round(e, prec)]
    return r

# modifies doc AND returns props
def iterate_properties(doc,node):
    if not hasattr(doc,"properties"):
        doc["properties"] = {}
    props = doc["properties"]
    if node:
        for k in node.keys():
            if not k.startswith('_') and not k.startswith('cycles'):
                props[k] = node.get(k)
    return props

def iterate_node(scene, obj, context, nodes):
    node = {}
    
    if obj.type == "MESH":
        
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
        # unknown type
        node = {
            'name': obj.name,
            'type': 'node',
            'matrix': mat(obj.matrix_local)
        }
    
    if obj.type != "MESH":
        iterate_properties(node, obj.data)
    iterate_properties(node, obj)
    
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
        mesh.calc_tessface()
        has_tangents = False
        has_fade = False
        try:
            mesh.calc_tangents()
            has_tangents = True
        except:
            pass
        mesh.update(calc_edges=True, calc_tessface=True)
        vertices = []
        normals = []
        tangents = []
        indices = []
        wrap = []
        fade = []
        idx = 0
        mats = []
        images = []

        # for face in mesh.tessfaces:
        #     mat += [mesh.uv_textures[0].data[face.index].image.name]
        has_uv = False

        # for m in mat:
        for face in mesh.polygons:
            if bool(mesh.uv_layers.active):
                has_uv = True
            for vert, i in zip(face.vertices, face.loop_indices):
            # for i in face.loop_indices:
                vertices += rounded(list(mesh.vertices[vert].co.to_tuple()),prec)
                # normals += rounded(list(mesh.vertices[vert].normal.to_tuple()),prec)
                normals += rounded(list(mesh.loops[i].normal.to_tuple()),prec)
                if has_tangents:
                    tangents += rounded(list(mesh.loops[i].tangent.to_tuple() + (mesh.loops[i].bitangent_sign,)),prec)
                if has_uv:
                    wrap += rounded(invert_uv(list(mesh.uv_layers.active.data[i].uv.to_tuple())),prec)
                else:
                    wrap += [0.0, 0.0]

                # for j in range(3):
                if mesh.vertex_colors.active:
                    c = round(mesh.vertex_colors.active.data[i].color[0],prec)
                    c = 1.0 - c
                    fade += [c]
                    if c > 0.01:
                        # if this is never marked as true, fade values aren't included in file since they're all 0's
                        has_fade = True
                else:
                    fade += [0.0]
            
            if len(mesh.materials)>0 and hasattr(mesh.materials[face.material_index].active_texture,"image"):
                images += [basename(mesh.materials[face.material_index].active_texture.image.filepath)]
            elif mesh.uv_textures.active and mesh.uv_textures.active.data[face.index].image:
                images += [basename(mesh.uv_textures.active.data[face.index].image.filepath)]
            else:
                images += [""] # no image
           
        # if mesh.tessface_vertex_colors:
        #     for e in mesh.tessface_vertex_colors.active.data:
        #         fade += rounded(list(e.color1.to_tuple()),prec)
        #         fade += rounded(list(e.color2.to_tuple()),prec)
        #         fade += rounded(list(e.color3.to_tuple()),prec)
        
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
            'tangents': tangents,
            'indices': indices,
            'wrap': wrap,
            'fade': fade,
            'properties': iterate_properties({}, obj.data)
        }

        # TODO split doc data based on assigned images
        docs = {}
        while vertices:
            if not images[0] in docs:
                docs[images[0]] = {}
            
                docs[images[0]]['indices'] = []
                docs[images[0]]['vertices'] = []
                docs[images[0]]['normals'] = []
                docs[images[0]]['tangents'] = []
                docs[images[0]]['wrap'] = []
                docs[images[0]]['fade'] = []

            docs[images[0]]['vertices'] += vertices[0:9]
            vertices = vertices[9:]
            if indices:
                docs[images[0]]['indices'] += indices[0:3]
                indices = indices[3:]
            if normals:
                docs[images[0]]['normals'] += normals[0:9]
                normals = normals[9:]
            if tangents:
                docs[images[0]]['tangents'] += tangents[0:12]
                tangents = tangents[12:]
            if wrap and has_uv:
                docs[images[0]]['wrap'] += wrap[0:6]
                wrap = wrap[6:]
            if fade and has_fade:
                docs[images[0]]['fade'] += fade[0:3]
                fade = fade[3:]

            images = images[1:]
        
        for k,subdoc in docs.items():
            subdoc['num_indices'] = len(subdoc['indices'])
            subdoc['indices'] = " ".join(str(i) for i in subdoc['indices'])
            
            subdoc['num_vertices'] = len(subdoc['vertices']) // 3
            subdoc['vertices'] = " ".join(str(i) for i in subdoc['vertices'])
            
            subdoc['num_normals'] = len(subdoc['normals']) // 3
            subdoc['normals'] = " ".join(str(i) for i in subdoc['normals'])
            
            subdoc['num_tangents'] = len(subdoc['tangents']) // 4
            subdoc['tangents'] = " ".join(str(i) for i in subdoc['tangents'])
            
            subdoc['num_wrap'] = len(subdoc['wrap']) // 2
            subdoc['wrap'] = " ".join(str(i) for i in subdoc['wrap'])
            
            subdoc['num_fade'] = len(subdoc['fade'])
            subdoc['fade'] = " ".join(str(i) for i in subdoc['fade'])
 
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
        iterate_properties(doc, obj)
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

