import bpy
import mathutils

def save(operator, ctx, filepath=""):
    bps.ops.object.duplicate()
    bps.ops.object.mode_set(mode="OBJECT")
    bps.ops.object.modifier_add(type="TRIANGULATE")
    bps.ops.object.modifier_apply(apply_as="DATA", modifier="Triangulate")
    bps.ops.object.delete()

    return {"FINISHED"}

