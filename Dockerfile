FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Minimal common runtime deps (add more if ldd shows missing libs)
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libstdc++6 \
    libgcc-s1 \
    libc6 \
 && rm -rf /var/lib/apt/lists/*

# Copy server runtime output into expected folder layout
COPY ArchivedBuilds/LinuxServer/ /opt/FPSDemo/

# Run as non-root
RUN useradd -m -u 10001 serveruser \
 && chown -R serveruser:serveruser /opt/FPSDemo

USER serveruser
WORKDIR /opt/FPSDemo

EXPOSE 7777/udp
EXPOSE 7777/tcp

# Ensure script is executable (use root briefly)
USER root
RUN chmod +x /opt/FPSDemo/FPSDemoServer.sh
USER serveruser

ENTRYPOINT ["./FPSDemoServer.sh"]
