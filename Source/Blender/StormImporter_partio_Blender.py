import bpy
import partio

# Custom property group to store the file path
class PartioImporterProperties(bpy.types.PropertyGroup):
    filepath: bpy.props.StringProperty(
        name="File Path",
        description="Path to the Partio file",
        subtype='FILE_PATH'
    )

def import_partio(context):
    props = context.scene.partio_importer_props

    # Open the Partio file
    p = partioParticles()
    p.load(props.filepath)

    # Create a new particle system
    bpy.ops.object.particle_system_add()

    # Access the active particle system
    psys = bpy.context.object.particle_systems.active
    particles = psys.particles

    # Loop through the particles in the Partio file
    for i in range(p.numParticles()):
        # Get particle data
        pos = p.getPosition(i)
        pid = p.getID(i)
        velocity = p.getVelocity(i)
        time = p.getTime(i)

        # Add a new particle to the Blender particle system
        particle = particles.add()
        particle.location = pos
        particle.velocity = velocity

        # Assign custom properties if needed
        particle["id"] = pid
        particle["time"] = time

        # Set keyframes for particle position based on time
        frame_start = bpy.context.scene.frame_start
        frame_end = bpy.context.scene.frame_end
        frames = frame_end - frame_start

        for frame in range(frame_start, frame_end + 1):
            t = (frame - frame_start) / frames

            if frame == frame_end:
                # Last frame, set particle position directly
                particle.location = pos
            else:
                # Interpolate particle position
                next_pos = p.getPosition(i + 1)
                interpolated_pos = [(1 - t) * p1 + t * p2 for p1, p2 in zip(pos, next_pos)]
                particle.location = interpolated_pos

            # Set keyframe for particle position
            particle.keyframe_insert(data_path="location", frame=frame)

    # Update the particle system
    bpy.context.object.update_from_editmode()

class partioParticles:
    def __init__(self):
        self.particleData = None

    def load(self, filepath):
        self.particleData = partio.read(filepath)

    def numParticles(self):
        return self.particleData.numParticles()

    def getPosition(self, index):
        return self.particleData.getXYZ(index)

    def getID(self, index):
        return self.particleData.getAttribute("id").data[index][0]

    def getVelocity(self, index):
        return self.particleData.getAttribute("velocity").data[index]

    def getTime(self, index):
        return self.particleData.getAttribute("time").data[index][0]

# UI Panel and Operator (same as before)

def register():
    bpy.utils.register_class(PartioImporterProperties)
    bpy.types.Scene.partio_importer_props = bpy.props.PointerProperty(type=PartioImporterProperties)
    bpy.utils.register_class(PartioImporterPanel)
    bpy.utils.register_class(PartioImportOperator)

def unregister():
    bpy.utils.unregister_class(PartioImportOperator)
    bpy.utils.unregister_class(PartioImporterPanel)
    bpy.utils.unregister_class(PartioImporterProperties)
    del bpy.types.Scene.partio_importer_props

# Register the classes
if __name__ == "__main__":
    register()
