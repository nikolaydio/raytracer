#pragma once

#include "types.h"
#include <glm/gtx/constants.hpp>
namespace rt {
	namespace core {

		class BxDF {
		public:
			//standard BxDF function
			virtual Spectrum evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const = 0;
			virtual Spectrum evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const;

			virtual float calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const;
		};
		class BRDF : public BxDF {
		public:

		};
		class LambertianBRDF : public BRDF {
			Spectrum _R; //reflectance spectrum

		public:
			LambertianBRDF(Spectrum R) : _R(R) {}

			Spectrum evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const {
				return _R * glm::one_over_pi<float>();
			}
		};
		class SpecularReflectionBRDF : public BRDF {
			Spectrum _R; //reflectance spectrum
		public:
			SpecularReflectionBRDF(Spectrum R) : _R(R) {}

			Spectrum evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const {
				return Spectrum(0, 0, 0);
			}
			Spectrum evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const {
				*incident_w = glm::vec3(-outgoing_w.x, -outgoing_w.y, outgoing_w.z);
				*pdf = 1;
				return _R / glm::abs(incident_w->z);
			}
		};

		class BSDF : public BxDF {
			static const int MAX_BRDFs = 8;
			BRDF* _brdfs[MAX_BRDFs];
			int _brdf_count;

			glm::vec3 _normal;
			
			glm::vec3 world_to_local(glm::vec3 world) const;
		public:
			BSDF(glm::vec3 normal);
			void add_brdf(BRDF* brdf);

			virtual Spectrum evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const;
			Spectrum evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const;

			virtual float calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const;
		};
	}
}