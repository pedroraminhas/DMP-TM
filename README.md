# DMP-TM

DMP-TM is an HyTM algorithm that exploits a key novel idea: leveraging operating system-level memory protection mechanisms to detect conflicts between HTM and STM transactions. This innovative design allows for employing highly scalable ORec-based STM implementations, while avoiding any instrumentation on the HTM path. DMP-TM demonstrated robust performance in an extensive evaluation, achieving gains of up to ∼20× when compared to state of the art HyTM systems.
