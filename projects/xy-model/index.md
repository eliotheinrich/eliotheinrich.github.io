---
layout: home 
title: XY Model
author_profile: true
permalink: /projects/xy-model/
---

The XY model is the prototypical example of a topological phase transition. Below is an interactive applet that I wrote
which performs Markov-chain Monte Carlo simulations of it. Below is a description of the significance of the model and 
a related research project of mine.

# Interactive Applet
<div class="canvas-container">
  <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
</div>

<div class="controls-columns">
  <!-- Left column: Temperature + Cluster update -->
  <div class="controls-column">
    <!-- Temperature slider -->
    <div class="control-block">
      <label for="slider">Temperature:</label>
      <input type="range" id="slider" min="0" max="100" value="50">
      <span id="sliderValue">1.0</span>
    </div>

    <!-- Cluster update button -->
    <div class="control-block">
      <button id="callBtn">Cluster updates</button>
    </div>
  </div>

  <!-- Right column: Magnetic field -->
  <div class="controls-column">
    <div class="control-block">
      <label>Magnetic Field:</label>
      <canvas id="fieldCanvas" width="100" height="100"></canvas>
      <div class="field-readout">
        <span>Magnitude: <span id="fieldMag">0</span></span>  
        <span>Direction: <span id="fieldDir">0°</span></span>
      </div>
    </div>
  </div>
</div>

<style>
  .canvas-container { 
    width: 100%; 
    max-width: 800px; 
    margin: 0 auto 30px auto; 
  }
  #canvas { 
    width: 100%; 
    height: auto; 
    display: block; 
  }

  .controls-columns {
    display: flex;
    justify-content: center;
    gap: 40px;
    flex-wrap: wrap;
  }
  
  .controls-column {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 20px;
    min-width: 200px;
  }
  
  .control-block {
    display: flex;
    flex-direction: column;
    align-items: center;
    text-align: center;
    width: 100%;
  }
  
  .control-block label {
    margin-bottom: 8px; 
    font-weight: 600;
    font-size: 14px; 
    display: block;
  }
  
  .field-readout {
    display: flex;
    justify-content: center;
    gap: 15px;
    margin-top: 10px;
  }
  
  .field-readout span {
    font-family: monospace;
    font-size: 13px;
    text-align: center;
  }

  /* Fixed width for dynamic text elements */
  #sliderValue {
    display: inline-block;
    width: 40px;
    text-align: center;
    font-family: monospace;
    font-weight: 600;
    margin-top: 5px;
  }

  #fieldMag, #fieldDir {
    display: inline-block;
    width: 50px;
    text-align: center;
    font-weight: 600;
  }

  button { 
    padding: 12px 24px; 
    font-size: 16px; 
    font-weight: 600;
    border: none;
    border-radius: 10px;
    background: linear-gradient(135deg, #4a90e2, #357ab7);
    color: white;
    cursor: pointer;
    box-shadow: 0 4px 8px rgba(0,0,0,0.15);
    transition: background 0.3s ease, transform 0.15s ease, box-shadow 0.3s ease;
    width: auto;
    min-width: 150px;
  }

  button:hover { 
    background: linear-gradient(135deg, #5aa0f2, #4688c7);
    box-shadow: 0 6px 12px rgba(0,0,0,0.2);
    transform: translateY(-2px);
  }

  button:active {
    background: linear-gradient(135deg, #3a78c2, #2e639e);
    transform: translateY(0);
    box-shadow: 0 3px 6px rgba(0,0,0,0.15);
  }

  input[type=range] {
    -webkit-appearance: none;
    width: 180px;
    height: 6px;
    border-radius: 5px;
    background: linear-gradient(90deg, #4a90e2, #357ab7);
    outline: none;
    transition: background 0.3s ease;
    cursor: pointer;
    margin-bottom: 5px;
  }

  input[type=range]:hover {
    background: linear-gradient(90deg, #5aa0f2, #4688c7);
  }

  /* Chrome, Safari, Edge */
  input[type=range]::-webkit-slider-thumb {
    -webkit-appearance: none;
    appearance: none;
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: white;
    border: 2px solid #357ab7;
    box-shadow: 0 2px 4px rgba(0,0,0,0.2);
    cursor: pointer;
    transition: transform 0.15s ease, border-color 0.3s ease;
  }

  input[type=range]::-webkit-slider-thumb:hover {
    transform: scale(1.1);
    border-color: #5aa0f2;
  }

  /* Firefox */
  input[type=range]::-moz-range-thumb {
    width: 18px;
    height: 18px;
    border-radius: 50%;
    background: white;
    border: 2px solid #357ab7;
    box-shadow: 0 2px 4px rgba(0,0,0,0.2);
    cursor: pointer;
    transition: transform 0.15s ease, border-color 0.3s ease;
  }

  input[type=range]::-moz-range-thumb:hover {
    transform: scale(1.1);
    border-color: #5aa0f2;
  }

  /* Track for Firefox */
  input[type=range]::-moz-range-track {
    height: 6px;
    border-radius: 5px;
    background: linear-gradient(90deg, #4a90e2, #357ab7);
  }

  #fieldCanvas { 
    display: block;
    border: 1px solid #ccc; 
    border-radius: 50%; 
    cursor: crosshair; 
    margin: 10px auto;
  }
</style>

<script>
var Module = {
  canvas: document.getElementById('canvas'),
  onRuntimeInitialized: function () {
      const button = document.getElementById('callBtn');
      button.addEventListener("click", function() {
          Module.ccall("button_callback", "void");
      });

      const slider = document.getElementById('slider');
      const display = document.getElementById('sliderValue');

      slider.addEventListener("input", function() {
          const maxTemp = 2.0;
          const minTemp = 0.0;

          const idx = parseInt(slider.value);
          const val = minTemp + idx * (maxTemp - minTemp) / 100.0;
          console.log(`Calling slider_callback. Val = ${val}`);
          display.textContent = val.toFixed(2);
          Module.ccall("slider_callback", "void", ["number"], [val]);
      });
  }
};

// --- Radial magnetic field controller ---
const canvasField = document.getElementById("fieldCanvas");
const ctx = canvasField.getContext("2d");
const magSpan = document.getElementById("fieldMag");
const dirSpan = document.getElementById("fieldDir");

const center = { x: canvasField.width / 2, y: canvasField.height / 2 };
const radius = canvasField.width / 2 - 5;
let fieldVec = { x: 0, y: 0 };

function drawController() {
  ctx.clearRect(0, 0, canvasField.width, canvasField.height);
  // Outer circle
  ctx.beginPath();
  ctx.arc(center.x, center.y, radius, 0, Math.PI * 2);
  ctx.stroke();
  // Vector
  ctx.beginPath();
  ctx.moveTo(center.x, center.y);
  ctx.lineTo(center.x + fieldVec.x, center.y + fieldVec.y);
  ctx.strokeStyle = "red";
  ctx.stroke();
  ctx.strokeStyle = "black";
}

function updateField(x, y) {
  let dx = x - center.x;
  let dy = y - center.y;
  let mag = Math.sqrt(dx*dx + dy*dy);
  if (mag > radius) {
    dx *= radius / mag;
    dy *= radius / mag;
    mag = radius;
  }
  fieldVec = { x: dx, y: dy };
  let magnitude = (mag / radius).toFixed(2);
  let angle = (Math.atan2(dy, dx) * 180 / Math.PI).toFixed(1);
  magSpan.textContent = magnitude;
  dirSpan.textContent = angle + "°";

  // Send normalized values to WASM backend
  Module.ccall("radial_callback", "void", ["number","number"], [magnitude, angle]);

  drawController();
}

canvasField.addEventListener("mousedown", e => {
  function move(ev) {
    const rect = canvasField.getBoundingClientRect();
    updateField(ev.clientX - rect.left, ev.clientY - rect.top);
  }
  move(e);
  window.addEventListener("mousemove", move);
  window.addEventListener("mouseup", () => {
    window.removeEventListener("mousemove", move);
  }, { once: true });
});

drawController();
</script>

<script src="{{ '/assets/js/xymodel.js' | relative_url }}"></script>

In this applet, the color the pixels indicates the direction of the spin; black corresponds to pointing up, white corresponds to down, and gray is in-between.
'+' and '-' symbols indicate vortices and antivortices, respectively, as defined by the winding number about the corresponding plaquette.

You can tune the temperature of the model, as well as the strength/direction of the external magnetic field. Press the button to switch from Metropolis-Hastings
updates to Wolff-cluster updates (augmented with a ['ghost spin'](https://arxiv.org/abs/1805.04019). See how it affects the convergence at low temperatures!

# The Model

The so-called XY model is described by the Hamiltonian

$$H_{XY} = -J\sum\limits_{\langle i, j\rangle} \mathbf{S}_i \cdot \mathbf{S}_j - \mathbf{B} \cdot \sum\limits_{i} \mathbf{S}$$

where $\mathbf{S}_i = (\cos(\theta_i), \sin(\theta_i))$ is a $2d$ classical spin vector, and $\langle i, j \rangle$ runs over nearest
neighbor spins in a $2d$ square lattice. When $\mathbf{B} = 0$, this model has a continuous $O(2)$ rotational symmetry, which prevents a low-temperature
magnetically ordered phase from existing according to the Mermin-Wagner theorem. 

Remarkably, this model nonetheless exhibits a phase transition from a high-temperature disordered phase to a low-temperature quasi-ordered phase. 
This transition is driven by the proliferation of topological defects, i.e. vortices and antivortices, in the spin texture. Vortices prevent the spins from aligning 
into a homogeneous ordered state, therefore inducing an energy 'cost'. At the same time, a vortex cannot be removed from the system through local operations unless it comes 
in contact with an antivortex pair with opposite winding number.

At low temperature, the binding energy cost of vortex-antivortex pairs is too large, resulting in no topological defects forming. As the temperature increases, the system experiences a proliferation of defects 
unbinding from their partners, thereby destroying the magnetic ordering. This first example of a topological phase transition was theorized by
Berezinski, Kosterlitz, and Thoulles, and the transition bears their name (BKT). Play around with my interactive applet below to see it in action!

I originally became interested in the XY model as it relates to a real materials, europium-based A-type antiferromagnets.
These materials were observed to exhibit an anomalously high magnetoresistance (a materials' electrical resistance as a 
function of applied external magnetic field) [at certain temperatures](https://arxiv.org/abs/2102.00204). A proposed mechanism
for the large magnetoresistance was [electron scattering on magnetic defects](https://arxiv.org/abs/2104.12291), as when 
vortex-antivortex pairs become unbound in the XY model, hinting that the source of the magnetoresistance lies in the system's magnetic
fluctuations. 

At this point, I became involved with the investigation. While the currently accepted magnetic model for this system had some similarities
to the XY model, it was signficantly more complicated. It was unclear if magnetic topological defects could even exist, much less
lead to a topological phase transition. I constructed the magnetic model and, through large-scale numerical effort using Markov-chain Monte Carlo
techniques, was able to demonstrate that a.) the magnetic model did exhibit a topological phase transition at appropriate model parameters, and b.) 
the transition was appropriately sensitive to experimentally tunable parameters like external field. You can find the [full work here](https://arxiv.org/abs/2207.02361),
and the code for this applet (and many other MCMC projects) can be found on [my github](https://github.com/eliotheinrich/MonteCarlo).
