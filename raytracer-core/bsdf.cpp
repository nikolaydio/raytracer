#include "bsdf.h"
#include <glm\glm.hpp>

namespace rt {
	namespace core {
		//Utility
		glm::vec3 cosine_sample_hemisphere(float u1, float u2) {
			return glm::vec3(0, 0, 0);
		}

		glm::vec3 uniform_sample_hemisphere(float u1, float u2) {
			float z = u1;
			float r = sqrtf(glm::max(0., 1. - z*z));
			float phi = 2 * glm::pi<float>() * u2;
			float x = r * cosf(phi);
			float y = r * sinf(phi);
			return glm::vec3(x, y, z);
		}
		float uniform_hemisphere_pdf() {
			return glm::one_over_pi <float>() / 2.0f;
		}

		//Impl
		Spectrum BxDF::evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const {
			//Better use cosine_sample when I learn the required math
			*incident_w = uniform_sample_hemisphere(u1, u2);
			if (outgoing_w.z < 0.) incident_w->z *= -1.f;
			*pdf = calc_pdf(outgoing_w, *incident_w);
			return evaluate_f(outgoing_w, *incident_w);
		}
		float BxDF::calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const {
			return uniform_hemisphere_pdf();
		}


		BSDF::BSDF(glm::vec3 normal) {
			_normal = normal;
			_brdf_count = 0;
		}
		void BSDF::add_brdf(BRDF* brdf, float scale) {
			assert(_brdf_count < MAX_BRDFs);
			_brdf_scales[_brdf_count] = scale;
			_brdfs[_brdf_count++] = brdf;
		}
		glm::vec3 BSDF::world_to_local(glm::vec3 world) const {
			glm::vec3 random_vec(0.85577678, 0.503440032, 0.119139549);
			glm::vec3 tn = glm::normalize(glm::cross(_normal, random_vec));
			glm::vec3 sn = glm::normalize(glm::cross(_normal, tn));

			return glm::vec3(glm::dot(world, sn), glm::dot(world, tn), glm::dot(world, _normal));
		}
		glm::vec3 BSDF::local_to_world(glm::vec3 local) const {
			glm::vec3 random_vec(0.85577678, 0.503440032, 0.119139549);
			glm::vec3 tn = glm::normalize(glm::cross(_normal, random_vec));
			glm::vec3 sn = glm::normalize(glm::cross(_normal, tn));

			return glm::vec3(sn.x * local.x + tn.x * local.y + _normal.x * local.z,
							sn.y * local.x + tn.y * local.y + _normal.y * local.z,
							sn.z * local.x + tn.z * local.y + _normal.z * local.z);
		}
		Spectrum BSDF::evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const {
			//translate world coords to local hemisphere coords
			glm::vec3 local_outgoing = world_to_local(outgoing_w);
			glm::vec3 local_incident = world_to_local(incident_w);

			Spectrum f;
			for (int i = 0; i < _brdf_count; ++i) {
				f += _brdfs[i]->evaluate_f(local_outgoing, local_incident) * _brdf_scales[i];
			}
			return f;
		}
		Spectrum BSDF::evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const {
			assert(u1 < 1.0);
			int brdf_num = glm::round(u1 * (_brdf_count-1));
			BRDF* brdf = _brdfs[brdf_num];

			glm::vec3 local_outgoing = world_to_local(outgoing_w);
			glm::vec3 local_incident;
			Spectrum f = brdf->evaluate_sample_f(local_outgoing, &local_incident, u1, u2, pdf) * _brdf_scales[brdf_num];

			*incident_w = local_to_world(local_incident);
			return f;
		}

		float BSDF::calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const {
			glm::vec3 local_outgoing = world_to_local(outgoing_w);
			glm::vec3 local_incident = world_to_local(incident_w);
			float pdf = 0;
			for (int i = 0; i < _brdf_count; ++i) {
				pdf += _brdfs[i]->calc_pdf(local_outgoing, local_incident) * _brdf_scales[i];
			}
			return pdf / _brdf_count;
		}

	}
}