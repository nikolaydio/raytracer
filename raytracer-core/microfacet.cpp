#include "microfacet.h"

namespace rt {
	namespace core {
		float G(const glm::vec3 outgoing_w, glm::vec3 incident_w, glm::vec3 wh) {
			float n_dot_wh = glm::abs(wh.z);
			float n_dot_out = glm::abs(outgoing_w.z);
			float n_dot_in = glm::abs(incident_w.z);

			float wh_dot_out = glm::dot(wh, outgoing_w);

			return glm::min(1.0f, glm::min((2.f * n_dot_wh * n_dot_out),
										(2.f * n_dot_wh * n_dot_in)));
		}
		//based on torrance-sparrow work
		Spectrum Microfacet::evaluate_f(glm::vec3 outgoing_w, glm::vec3 incident_w) const {
			float cos_thetha_out = glm::abs(outgoing_w.z);
			float cos_thetha_in = glm::abs(incident_w.z);

			if (cos_thetha_in == 0.f || cos_thetha_out == 0.f) return Spectrum(0, 0, 0);

			glm::vec3 wh = glm::normalize(outgoing_w + incident_w);
			float cos_theta_h = glm::dot(incident_w, wh);
			return R * distribution->D(wh) * G(outgoing_w, incident_w, wh) /
				(4.f * cos_thetha_out * cos_thetha_in);

		}
		Spectrum Microfacet::evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const {
			distribution->evaluate_sample_f(outgoing_w, incident_w, u1, u2, pdf);
			return evaluate_f(outgoing_w, *incident_w);
		}

		float Microfacet::calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const {
			return distribution->calc_pdf(outgoing_w, incident_w);
		}

		float Blinn::D(const glm::vec3 wh) const {
			float costhethah = glm::abs(wh.z);
			return (exponent + 2) * (glm::one_over_pi<float>() / 2.f) * glm::pow(costhethah, exponent);
		}
		void Blinn::evaluate_sample_f(glm::vec3 outgoing_w, glm::vec3* incident_w, float u1, float u2, float* pdf) const {
			float costheta = glm::pow(u1, 1.f / (exponent + 1));
			float sintheta = glm::sqrt(glm::max(0.f, 1.0f - costheta*costheta));
			float phi = u2 * 2.0f * glm::pi<float>();

			glm::vec3 wh = spherical_direction(sintheta, costheta, phi);

			*incident_w = -outgoing_w + 2.f * glm::dot(outgoing_w, wh) * wh;

			float blinn_pdf = ((exponent + 1.f) * glm::pow(costheta, exponent)) /
				(2.f * glm::pi<float>() * 4.f * glm::dot(outgoing_w, wh));
			if (glm::dot(outgoing_w, wh) <= 0.f) {
				blinn_pdf = 0.;
			}
			*pdf = blinn_pdf;
		}

		float Blinn::calc_pdf(const glm::vec3 &outgoing_w, const glm::vec3 &incident_w) const {
			glm::vec3 wh = glm::normalize(outgoing_w + incident_w);
			float costheta = glm::abs(wh.z);

			float blinn_pdf = ((exponent + 1.f) * glm::pow(costheta, exponent)) /
				(2.f * glm::pi<float>() * 4.f * glm::dot(outgoing_w, wh));
			if (glm::dot(outgoing_w, wh) <= 0.f) {
				blinn_pdf = 0.;
			}
			return blinn_pdf;
		}
	}
}