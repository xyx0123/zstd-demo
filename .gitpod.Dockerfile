# .gitpod.Dockerfile
FROM gcc:latest
RUN apt update && apt install -y libzstd-dev cmake
