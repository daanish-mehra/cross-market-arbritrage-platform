# Cross-Market Arbitrage Platform

Real-time arbitrage detection and execution across prediction markets (Polymarket, Kalshi, PredictIt). C++ backend for low-latency processing with planned modern frontend for visualization and monitoring.

## Tech Stack

C++11 backend with CMake build system and pthread for multi-threaded market data collection. Planned Next.js/TypeScript frontend with WebSocket communication for real-time updates.

## What

Real-time cross-market arbitrage detection platform that monitors Polymarket, Kalshi, and PredictIt to identify price discrepancies for the same events across different prediction markets.

## What/How

Continuously compares bid/ask prices across markets, calculates potential profit margins after fees, and flags opportunities exceeding a configurable threshold (default 1%). Uses function pointers for callbacks to notify when profitable arbitrage is detected.

## How

WebSocket clients connect to each market's API to stream live price data. Arbitrage engine stores latest market data in thread-safe structures, performs pairwise comparisons between markets for matching events, computes profit percentages, and triggers opportunity callbacks when profitable trades are found. All processing happens in C++ for low-latency detection.

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Run

```bash
./arbitrage-platform
```
