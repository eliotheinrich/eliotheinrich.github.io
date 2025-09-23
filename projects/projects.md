---
layout: home
title: My Projects
author_profile: true
permalink: /projects/
---

Welcome to my projects! Click on a project below to explore.

<div class="projects-grid">
  <a href="{{ '/projects/xy-model/' | relative_url }}">
    <img src="{{ '/assets/images/xy-model.png' | relative_url }}" alt="XY Model">
    <p>XY Model</p>
  </a>

  <a href="{{ '/projects/dla/' | relative_url }}">
    <img src="{{ '/assets/images/dla.png' | relative_url }}" alt="Diffusion Limited Aggregation">
    <p>Diffusion Limited Aggregation</p>
  </a>

  <a href="{{ '/projects/qutils/' | relative_url }}">
    <img src="{{ '/assets/images/qutils.png' | relative_url }}" alt="qutils">
    <p>qutils</p>
  </a>

  <a href="{{ '/projects/qrpm/' | relative_url }}">
    <img src="{{ '/assets/images/qrpm.png' | relative_url }}" alt="Quantum Raise and Peel">
    <p>Quantum Raise and Peel</p>
  </a>
</div>

<style>
.projects-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 20px;
  margin-top: 20px;
}

.projects-grid a {
  text-align: center;
  text-decoration: none;
  color: inherit;
}

.projects-grid img {
  width: 100%;
  height: 200px;
  object-fit: cover; /* crop and fill */
  border-radius: 8px;
  box-shadow: 0 2px 6px rgba(0,0,0,0.3);
  transition: transform 0.2s;
}

.projects-grid img:hover {
  transform: scale(1.05);
}

.projects-grid p {
  margin-top: 8px;
  font-weight: bold;
}
</style>

