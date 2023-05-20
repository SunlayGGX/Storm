import struct
from pathlib import Path

def read_particle_frames(filepath):
    frames = []

    with open(filepath, "rb") as file:
        # Read header
        magic_word = struct.unpack("<I", file.read(4))[0]
        if magic_word != 0xFFAABB77:
            print("Invalid file format.")
            return frames
            
        version = struct.unpack("<f", file.read(4))[0]
        print(f"File version: {version}")
        
        frame_count = struct.unpack("<Q", file.read(8))[0]
        print(f"Frame count: {frame_count}")
        
        particle_count = struct.unpack("<Q", file.read(8))[0]
        print(f"Particle count: {particle_count}")
        
        for _ in range(frame_count):
            time = struct.unpack("<f", file.read(4))[0]
            particles = read_particle_frame(file, particle_count)
            frame = ParticleFrame(time, particles)
            frames.append(frame)

    return frames


def read_particle_frame(file, particle_count):
    particles = []

    for _ in range(particle_count):
        # Read particle data
        particle_id = struct.unpack("<I", file.read(4))[0]
        position = struct.unpack("<3f", file.read(12))

        # Append particle data to the list
        particles.append((particle_id, position))

    return particles


def create_particle_animation(frames):
    for frame in frames:
        time = frame.time
        particles = frame.particles

        for particle_id, position in particles:
            # Perform your particle animation logic here
            # You can use the 'time' and 'position' values to animate particles in TouchDesigner

def import_particles(filepath):
    frames = read_particle_frames(filepath)
    if frames:
        create_particle_animation(frames)

# Set the filepath to your particle binary file
particle_filepath = "/path/to/your/particle_file.bin"

import_particles(particle_filepath)
