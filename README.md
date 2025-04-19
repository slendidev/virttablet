# virttablet

Virtual tablet driver for Linux.

## Usage

```sh
# Set bounds
echo 20 | sudo tee /sys/kernel/virttablet/minx
echo 4000 | sudo tee /sys/kernel/virttablet/maxx
echo 30 | sudo tee /sys/kernel/virttablet/miny
echo 3000 | sudo tee /sys/kernel/virttablet/maxy

# Set position
echo 2000 | sudo tee /sys/kernel/virttablet/x
echo 2000 | sudo tee /sys/kernel/virttablet/y
```

