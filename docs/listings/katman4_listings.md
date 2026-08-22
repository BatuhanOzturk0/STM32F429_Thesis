# Katman 4 — Code Listings for Thesis (Approach Section)

## Listing 4.X: Continuous sample processing inside IMU_Task

```c
if (dma_buf->half_ready_flag == 1 && dma_buf->half_processed == 0)
{
  DMA_Sample_t sample;
  DMA_Handler_ParseSample(dma_buf->raw_samples[0], &sample);
  halves_processed++;
  dma_buf->half_processed = 1;
}
```

Context: Runs inside IMU_Task's infinite loop, checking DMA half/full
buffer flags set by the ISR-driven acquisition chain (Layer 2).

## Listing 4.Y: Yielding control to prevent task starvation

```c
osDelay(1);
```

Context: Placed at the end of IMU_Task's loop body. Because IMU_Task
runs at osPriorityAboveNormal (higher than all other tasks), omitting
this call would starve defaultTask, DSP_Task, and UART_Task of CPU
time entirely, since the preemptive scheduler would never find a
reason to switch away from IMU_Task.
