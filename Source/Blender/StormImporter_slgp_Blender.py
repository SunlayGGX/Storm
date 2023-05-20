import bpy
import struct


class ParticleImportPanel(bpy.types.Panel):
    bl_label = "Particle Import"
    bl_idname = "OBJECT_PT_particle_import"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Particle Import"

    def draw(self, context):
        layout = self.layout
        scene = context.scene

        layout.prop(scene, "particle_filepath", text="Particle File")
        layout.operator("object.import_particles")


class OBJECT_OT_ImportParticles(bpy.types.Operator):
    bl_idname = "object.import_particles"
    bl_label = "Import Particles"

    def execute(self, context):
        scene = context.scene

        filepath = scene.particle_filepath
        particles = read_particle_system(filepath)

        if particles:
            create_particles(particles)
    
        return {'FINISHED'}



def read_particle_system(filepath):
    particles = []

    with open(filepath, "rb") as file:
        # Read header
        magic_word = struct.unpack("<I", file.read(4))[0]
        version = struct.unpack("<f", file.read(4))[0]
        particle_count = struct.unpack("<Q", file.read(8))[0]

        for _ in range(particle_count):
            # Read particle data
            particle_id = struct.unpack("<I", file.read(4))[0]
            time = struct.unpack("<f", file.read(4))[0]
            position = struct.unpack("<3f", file.read(12))

            # Append particle data to the list
            particles.append((particle_id, time, position))

    return particles


def create_particles(particles):
    # Create particles in Blender
    for particle_id, time, position in particles:
        obj = bpy.data.objects.new("Particle", None)
        bpy.context.collection.objects.link(obj)
        obj.location = position

        # Create animation keyframes
        obj.animation_data_create()
        obj.animation_data.action = bpy.data.actions.new(name="ParticleAnimation")
        obj.keyframe_insert(data_path="location", frame=round(time))

        # Check if the particle has previous keyframes
        if obj.animation_data.action.fcurves.find("location", index=0) is not None:
            # Interpolate position with previous keyframe
            obj.keyframe_insert(data_path="location", frame=round(time) - 1)
            obj.animation_data.action.fcurves.find("location", index=0).keyframe_points[-1].interpolation = 'LINEAR'
            obj.animation_data.action.fcurves.find("location", index=1).keyframe_points[-1].interpolation = 'LINEAR'
            obj.animation_data.action.fcurves.find("location", index=2).keyframe_points[-1].interpolation = 'LINEAR'

        # Set the interpolation mode of the first keyframe to LINEAR
        obj.animation_data.action.fcurves.find("location", index=0).keyframe_points[0].interpolation = 'LINEAR'
        obj.animation_data.action.fcurves.find("location", index=1).keyframe_points[0].interpolation = 'LINEAR'
        obj.animation_data.action.fcurves.find("location", index=2).keyframe_points[0].interpolation = 'LINEAR'

    # Set the current frame to the first keyframe
    bpy.context.scene.frame_set(0)


def register():
    bpy.types.Scene.particle_filepath = bpy.props.StringProperty(subtype='FILE_PATH')
    bpy.utils.register_class(ParticleImportPanel)
    bpy.utils.register_class(OBJECT_OT_ImportParticles)


def unregister():
    bpy.utils.unregister_class(ParticleImportPanel)
    bpy.utils.unregister_class(OBJECT_OT_ImportParticles)
    del bpy.types.Scene.particle_filepath


if __name__ == "__main__":
    register()
