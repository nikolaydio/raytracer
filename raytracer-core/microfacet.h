#pragma once
#include "bsdf.h"

namespace rt {
	namespace core {

		//2 utility functions
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi,
			const glm::vec3 x, const glm::vec3 y, const glm::vec3 z);
		glm::vec3 spherical_direction(float sintheta, float costheta, float phi);

		//to thetha(x) and phi(y)
		glm::vec2 spherical_angles(const glm::vec3 dir);
		//to sintheta(x), costheta(y) and phi(z)
		glm::vec3 spherical_angles_tri(const glm::vec3 dir);


		class MicrofacetDistribution {
		public:
			virtual float D(const glm::vec3 wh) const = 0;

			virtual void evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const = 0;

			virtual float calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const = 0;
		};
		class Blinn : public MicrofacetDistribution {
			float exponent;
		public:
			Blinn(float e) : exponent(e) {}
			~Blinn() {}
			virtual float D(const glm::vec3 wh) const;
			virtual void evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const;

			virtual float calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const;
		};
		class Microfacet : public BRDF {
			MicrofacetDistribution* distribution;
			Spectrum R;

		public:
			Microfacet(Spectrum r, MicrofacetDistribution* d) :
				distribution(d), R(r) {}
			~Microfacet() {}

			virtual Spectrum evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const;
			virtual Spectrum evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const;

			virtual float calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const;
		};
	}
}