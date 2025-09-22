---
layout: home 
title: Diffusion Limited Aggregation
author_profile: true
permalink: /projects/dla/
---

Diffusion-limited aggregation is a dynamic process in which particles undergo Brownian motion (random walks) until coming
in contact with a structure, at which point it sticks to the structure. This process, surprisingly, forms a beautiful fractal structure. 

Play around with the applet!

# Interactive Applet

*Click and drag to create new aggregated particles!*
<div class="canvas-container">
  <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
</div>
<h2>Keyboard Controls</h2>
<ul>
  <li><strong>E:</strong> Increase target particle density</li>
  <li><strong>D:</strong> Decrease target particle density</li>
  <li><strong>C:</strong> Cycle through colors</li>
  <li><strong>R:</strong> Reset the simulation</li>
  <li><strong>P:</strong> Toggle display of particles</li>
  <li><strong>B:</strong> Toggle display of the bounding box</li>
  <li><strong>Spacebar:</strong> Pause or resume the simulation</li>
  <li><strong>Arrow Left:</strong> Toggle particle insertion on the left boundary</li>
  <li><strong>Arrow Right:</strong> Toggle particle insertion on the right boundary</li>
  <li><strong>Arrow Up:</strong> Toggle particle insertion on the top boundary</li>
  <li><strong>Arrow Down:</strong> Toggle particle insertion on the bottom boundary</li>
</ul>

<br>

<style>
  .canvas-container { width: 100%; max-width: 800px; margin: 0 auto; }
  #canvas { width: 100%; height: auto; display: block; }
  label { margin-right: 10px; }
</style>

<script>
var Module = {
  canvas: document.getElementById('canvas'),
  onRuntimeInitialized: function () { }
};
</script>
<script src="{{ '/assets/js/dla.js' | relative_url }}"></script>

# My Interest
I originally became interested in DLA due to a work by a colleague of mine discovered fractal structure in the logical operators
corresponding to parity operators in error correcting codes corresponding to random clifford circuits. He found that 
the logical operators had a fractal dimension very close to that of DLA fractals. It seems that this turned out to be a coincidence,
but I had a lot of fun writing this program to check it out!
