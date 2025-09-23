#pragma once

#include <cmath>
#include <map>
#include <iostream>

#include "Random.hpp"
#include "Simulator.hpp"
#include "Support.hpp"

enum BoundaryCondition { Periodic, Open };

typedef std::pair<uint32_t, int> Bond;

inline BoundaryCondition parse_boundary_condition(std::string s) {
  if (s == "periodic") {
    return BoundaryCondition::Periodic;
  } else if (s == "open") {
    return BoundaryCondition::Open;
  } else {
    std::string error_message = "Invalid boundary condition: " + s + "\n";
    throw std::invalid_argument(error_message);
  }
}

class MonteCarloSimulator : public Simulator {
  // Most basic Monte-Carlo model to be simulated must have some notion of energy
  // as well as a mutation data structure. Specifics must be supplied by child classes.
  public:
    MonteCarloSimulator()=default;

    MonteCarloSimulator(Params &params, uint32_t num_threads) : num_threads(num_threads) {
      temperature = get<double>(params, "temperature");

      if (temperature < 0) {
        throw std::runtime_error(std::format("Provided temperature {:.3f} is negative.", temperature));
      }
    }

    virtual ~MonteCarloSimulator();

    // Implement Simulator methods but introduce MonteCarlo methods
    virtual void timesteps(uint32_t num_steps) override {
      uint64_t num_updates = system_size()*num_steps;
      for (uint64_t i = 0; i < num_updates; i++) {
        generate_mutation();
        double dE = energy_change();

        double rf = randf();
        if (rf < std::exp(-dE/temperature)) {
          accept_mutation();
        } else {
          reject_mutation();
        }
      }
    }

    virtual void key_callback(const std::string& key) override;

    // To be overridden by child classes
    virtual double energy() const=0;
    virtual double energy_change()=0;
    virtual void generate_mutation()=0;
    virtual void accept_mutation()=0;
    virtual void reject_mutation()=0;
    virtual uint64_t system_size() const=0;

    double temperature;

  protected:

  private:
    uint32_t num_threads;
};

