---
layout: home 
title: "Quantum Raise and Peel"
author_profile: true
permalink: /projects/qrpm/
---

I devised the 'quantum raise and peel' model to emulate substrate growth in a quantum circuit model. It is an analogue to
the classical ['raise and peel' model](https://arxiv.org/abs/cond-mat/0301430), which applies update rules to a fluctuating 
substrate based on the local state of the substrate, either performing a 'raise' or deposition event, or a 'peel', or a 
evaporation event, in which a nonlocal layer of substrate is removed. This model exhibits a self-organized critical phase, 
i.e. a critical region which is an attractor for a dynamic system. 

# Interactive Applet

<div class="canvas-container">
  <canvas id="canvas" oncontextmenu="event.preventDefault()"></canvas>
</div>
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
<script src="{{ '/assets/js/qrpm.js' | relative_url }}"></script>


<title>Parameters</title>
<style>
  .controls-columns {
    display: flex;
    justify-content: center;
    align-items: flex-start; /* align both columns at the top */
    gap: 60px; /* extra breathing room between button and controller */
    flex-wrap: wrap;
    margin-top: 20px;
  }

  .controls-column {
    display: flex;
    flex-direction: column;
    align-items: center; /* ensures content in each column is horizontally centered */
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

  .param-readout {
    display: flex;
    justify-content: center;
    gap: 15px;
    margin-top: 10px;
  }

  .param-readout span {
    font-family: monospace;
    font-size: 13px;
    text-align: center;
  }

  #pu, #pm {
    display: inline-block;
    width: 50px;
    text-align: center;
    font-weight: 600;
  }

  #paramCanvas { 
    display: block;
    border: 1px solid #ccc; 
    cursor: crosshair; 
    margin: 10px auto;
  }

  #newStrategyBtn { 
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

  #newStrategyBtn:hover { 
    background: linear-gradient(135deg, #5aa0f2, #4688c7);
    box-shadow: 0 6px 12px rgba(0,0,0,0.2);
    transform: translateY(-2px);
  }

  #newStrategyBtn:active {
    background: linear-gradient(135deg, #3a78c2, #2e639e);
    transform: translateY(0);
    box-shadow: 0 3px 6px rgba(0,0,0,0.15);
  }
</style>

<div class="controls-columns">
  <!-- Left column: New strategy button -->
  <div class="controls-column">
    <div class="control-block">
      <button id="newStrategyBtn">New strategy</button>
    </div>
  </div>

  <!-- Right column: Parameter controller -->
  <div class="controls-column">
    <div class="control-block">
      <label>Parameter controller:</label>
      <canvas id="paramCanvas" width="100" height="100"></canvas>
      <div class="param-readout">
        <span>$p_u$: <span id="pu">0.00</span></span>  
        <span>$p_m$: <span id="pm">0.00</span></span>
      </div>
    </div>
  </div>
</div>

<script>
    const canvasField = document.getElementById("paramCanvas");
    const ctx = canvasField.getContext("2d");
    const xSpan = document.getElementById("pu");
    const ySpan = document.getElementById("pm");

    const center = { x: canvasField.width / 2, y: canvasField.height / 2 };
    const halfSize = canvasField.width / 2 - 5; // 5px padding from edges
    let paramVec = { x: 0, y: 0 };

    function drawController() {
        ctx.clearRect(0, 0, canvasField.width, canvasField.height);
        
        // Draw square boundary
        ctx.strokeStyle = "black";
        ctx.lineWidth = 1;
        ctx.strokeRect(center.x - halfSize, center.y - halfSize, halfSize * 2, halfSize * 2);
        
        // Draw param vector
        ctx.strokeStyle = "red";
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(center.x, center.y);
        ctx.lineTo(center.x + paramVec.x, center.y + paramVec.y);
        ctx.stroke();
    }

    function updateField(x, y) {
        let dx = x - center.x;
        let dy = y - center.y;
        
        // Constrain to square boundary
        if (Math.abs(dx) > halfSize) {
            dx = halfSize * Math.sign(dx);
        }
        if (Math.abs(dy) > halfSize) {
            dy = halfSize * Math.sign(dy);
        }
        
        paramVec = { x: dx, y: dy };
        
        // Normalize to [0, 1] range and display (flip Y to match typical coordinate system)
        const normalizedX = ((dx / halfSize) * 0.5 + 0.5).toFixed(2);
        const normalizedY = ((-dy / halfSize) * 0.5 + 0.5).toFixed(2);
        
        xSpan.textContent = normalizedX;
        ySpan.textContent = normalizedY;

        Module.ccall("radial_callback", "void", ["number","number"], [normalizedX, normalizedY]);

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

    const button = document.getElementById("newStrategyBtn");
    button.addEventListener("click", function() {
        Module.ccall("button_callback", "void");
    });

    // Initialize
    drawController();
</script>

In the quantum model, the entanglement serves as the substrate. At site $i$, the entanglement of the leftmost $i - 1$ qubits 
serves as the height $h_i$ of the substrate. In this picture, a local unitary applied to $h_i$ may locally effect the substrate,
while a projective measurement may nonlocally affect the entanglement substrate. The most extreme example is a GHZ state, where
a single measurement destroys all entanglement in the system. The last ingredient is a restriction on the update rules based on
the local state of the substrate, referred to as a feedback strategy. This can be done in a discrete number of ways. In my work, 
I investigated many different options, and found that the system could exhibit three phases.

The first phase is the 'frozen' volume law, in which the system is maximally entangled with no fluctuations. This phase is
typically protected by the update rules to experience no measurements in the bulk, thereby locking in a maximally entangled state.

The second phase is the paradigmatic volume law state, which experiences strong fluctuations (i.e. $\delta S \sim L^\gamma$), but
$S ~ O(L)$. This is the most typical volume-law observed in dynamic quantum systems.

Finally, the area law has entanglement which is $O(1)$ in the system size, and correspondingly small fluctuations. 

For a full description of the model, [see my paper](https://arxiv.org/pdf/2402.08605)! Below is an interactive app which
can be used to get an idea for the various entanglement phases. The entanglement substrate is displayed, and evolved subject
to the rules given in the paper. The measurement and unitary strengths can be tuned, along with the feedback strategy. All three
phases are observable here if you tune the parameters appropriately. Also try and find the diffusive transition between the area law and
frozen entanglemenet law, in which the protected regions exhibit diffusive growth until the entire system is maximally entangled. 
