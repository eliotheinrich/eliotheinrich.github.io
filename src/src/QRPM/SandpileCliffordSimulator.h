#pragma once

#include "Simulator.hpp"
#include "QuantumCHPState.h"

class SandpileCliffordSimulator : public Simulator {
	private:
    std::minstd_rand rng;
		uint32_t system_size;

		double unitary_prob;
		double mzr_prob;

		std::string boundary_condition;
		uint32_t feedback_mode;


		uint32_t unitary_qubits;
		uint32_t mzr_mode;
		
		std::vector<uint32_t> feedback_strategy;
		
		uint32_t initial_state;
		uint32_t scrambling_steps;

		void feedback(uint32_t q);

		void left_boundary();
		void right_boundary();

		void mzr(uint32_t i);
		void unitary(uint32_t i);
		
		void timestep();

		uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

    void set_feedback_strategy(int);

	public:
    void advance_feedback_strategy();
    void set_strengths(double pu, double pm);
		std::shared_ptr<QuantumCHPState> state;

		SandpileCliffordSimulator(Params& params);

		virtual void timesteps(uint32_t num_steps) override;
    virtual void draw() const override;
    virtual void key_callback(const std::string& key) override;
};
