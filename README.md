# Project-OpenCLe

## Development guide

![](./UML.png)

- OpenCLe: a singleton RAII object. Underlining implementation is OpenCL and contain all necessary recourse for OpenCL. When a Task is pushed, OpenCLe will automatic decide when and which devices run the task.

- Data: Data is a device-side memory

- Task: Task contains kernel code, arguments, dependency, and call back function. Once a task completed, a call back function will be called with same arguments as kernel. Task is only movable but not copiable.

- Dependency: the dependency of a task. Underlining implementation is a list of task pointer.