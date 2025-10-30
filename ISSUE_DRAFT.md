Title: Feature: Add thread-safe statistics collection

Body:
The task scheduler currently executes tasks but provides no visibility into its runtime behavior. Users need metrics to monitor performance and diagnose issues.

Requirements:
- Track number of completed tasks
- Track number of currently active worker threads
- Track current queue depth (pending tasks)
- Track task execution times (min, max, average)
- All statistics must be thread-safe
- Provide a method to retrieve current statistics snapshot
- Statistics should have minimal performance impact on task execution
