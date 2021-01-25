<p align="center">
  <a href="https://example.com/">
    <img src="https://ibu.edu.ba/wp-content/uploads/2019/08/IBU_213_57.png" alt="IBU" width=72 height=72>
  </a>

  <h3 align="center">[ IoT ][ RTOS ] Project</h3>

  <p align="center">
    A simple student project for TCP/IP communication between an Arduino board and a public server
  </p>
</p>


## Table of contents

- [Intro](#intro)
- [Status](#status)
- [Running the project](#running-the-project)
- [Bugs and feature requests](#bugs-and-feature-requests)

## Intro

The project uses a simulated Arduino Mega 2560 board with an ENC28J60 microchip for ethernet and a DHT11 module for sensor scans.<br>
The remote server is built in native GoLang with a simple UI made with React and Material-UI.

## Running the project

Download the repo via ``` git clone ```

```diff

```

## Bugs

The DHT11 module returns faulty timeout values. The cause is plausibly buggy RTOS scheduling on my part - further analysis is needed.
