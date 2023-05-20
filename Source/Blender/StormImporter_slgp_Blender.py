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
        frames = read_particle_frames(filepath)

        if frames:
            create_particle_animation(frames)

        return {'FINISHED'}


def read_particle_frames(filepath):
    frames = []

    with open(filepath, "rb") as file:
        # Read header
        magic_word = struct.unpack("<I", file.read(4))[0]
        version = struct.unpack("<f", file.read(4))[0]
        frame_count = struct.unpack("<Q", file.read(8))[0]

        for _ in range(frame_count):
            particles = read_particle_frame(file)
            frames.append(particles)

    return frames


def read_particle_frame(file):
    particles = []

    # Read frame header
    particle_count = struct.unpack("<Q", file.read(8))[0]

    for _ in range(particle_count):
        # Read particle data
        particle_id = struct.unpack("<I", file.read(4))[0]
        time = struct.unpack("<f", file.read(4))[0]
        position = struct.unpack("<3f", file.read(12))

        # Append particle data to the list
        particles.append((particle_id, time, position))

    return particles


def create_particle_animation(frames):
    # Create particles in Blender
    for frame_idx, particles in enumerate(frames):
        for particle_id, time, position in particles:
            obj = bpy.data.objects.get(f"Particle_{particle_id}")
            if obj is None:
                obj = bpy.data.objects.new(f"Particle_{particle_id}", None)
                bpy.context.collection.objects.link(obj)

            # Set particle position
            obj.location = position

            # Create animation keyframe for current frame
            obj.animation_data_create()
            obj.animation_data.action = bpy.data.actions.new(name=f"ParticleAnimation_{particle_id}")
            obj.keyframe_insert(data_path="location", frame=frame_idx)

            # Check if the particle has previous keyframes
            if frame_idx > 0 and obj.animation_data.action.fcurves.find("location", index=0) is not None:
                # Interpolate position with previous keyframe
                obj.keyframe_insert(data_path="location", frame=frame_idx - 1)
                obj.animation_data.action.fcurves.find("location", index=0).keyframe_points[-1].interpolation = 'LINEAR'
                obj.animation_data.action.fcurves.find("location", index=1).keyframe_points[-1].interpolation = 'LINEAR'
                obj.animation_data.action.fcurves.find("location", index=2).keyframe_points[-1].interpolation = 'LINEAR'

    # Set the current frame to the first frame
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
