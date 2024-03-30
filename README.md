# Socket Example
Playing around with sockets

# Build
```bash
# Create Build system, use source dir ., build dir build/
cmake -S . -B build

# Build Software
cmake --build build

# Run Client App
./build/client/client 

# Open different shell and run server 
./build/server/server
```