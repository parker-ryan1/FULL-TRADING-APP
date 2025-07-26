#include "black_scholes.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace hedgefund {
namespace options {

double BlackScholes::calculatePrice(const OptionParams& params) {
    if (params.time_to_expiry <= 0) {
        // At expiration
        if (params.is_call) {
            return std::max(0.0, params.spot_price - params.strike_price);
        } else {
            return std::max(0.0, params.strike_price - params.spot_price);
        }
    }
    
    double d1_val = d1(params);
    double d2_val = d2(params);
    
    double nd1 = normalCDF(d1_val);
    double nd2 = normalCDF(d2_val);
    
    if (params.is_call) {
        return params.spot_price * nd1 - 
               params.strike_price * std::exp(-params.risk_free_rate * params.time_to_expiry) * nd2;
    } else {
        return params.strike_price * std::exp(-params.risk_free_rate * params.time_to_expiry) * normalCDF(-d2_val) -
               params.spot_price * normalCDF(-d1_val);
    }
}

Greeks BlackScholes::calculateGreeks(const OptionParams& params) {
    Greeks greeks;
    
    if (params.time_to_expiry <= 0) {
        // At expiration, most Greeks are zero
        greeks.delta = (params.is_call && params.spot_price > params.strike_price) ? 1.0 : 0.0;
        if (!params.is_call && params.spot_price < params.strike_price) greeks.delta = -1.0;
        greeks.gamma = greeks.theta = greeks.vega = greeks.rho = 0.0;
        return greeks;
    }
    
    double d1_val = d1(params);
    double d2_val = d2(params);
    double nd1 = normalCDF(d1_val);
    double nd2 = normalCDF(d2_val);
    double npd1 = normalPDF(d1_val);
    
    // Delta
    if (params.is_call) {
        greeks.delta = nd1;
    } else {
        greeks.delta = nd1 - 1.0;
    }
    
    // Gamma (same for calls and puts)
    greeks.gamma = npd1 / (params.spot_price * params.volatility * std::sqrt(params.time_to_expiry));
    
    // Theta
    double theta_common = -(params.spot_price * npd1 * params.volatility) / (2 * std::sqrt(params.time_to_expiry));
    if (params.is_call) {
        greeks.theta = theta_common - params.risk_free_rate * params.strike_price * 
                      std::exp(-params.risk_free_rate * params.time_to_expiry) * nd2;
    } else {
        greeks.theta = theta_common + params.risk_free_rate * params.strike_price * 
                      std::exp(-params.risk_free_rate * params.time_to_expiry) * normalCDF(-d2_val);
    }
    greeks.theta /= 365.0; // Convert to daily theta
    
    // Vega (same for calls and puts)
    greeks.vega = params.spot_price * npd1 * std::sqrt(params.time_to_expiry) / 100.0; // Per 1% volatility change
    
    // Rho
    if (params.is_call) {
        greeks.rho = params.strike_price * params.time_to_expiry * 
                    std::exp(-params.risk_free_rate * params.time_to_expiry) * nd2 / 100.0;
    } else {
        greeks.rho = -params.strike_price * params.time_to_expiry * 
                    std::exp(-params.risk_free_rate * params.time_to_expiry) * normalCDF(-d2_val) / 100.0;
    }
    
    return greeks;
}

double BlackScholes::impliedVolatility(double market_price, const OptionParams& params, 
                                      double tolerance, int max_iterations) {
    double vol_low = 0.01;
    double vol_high = 5.0;
    double vol_mid = (vol_low + vol_high) / 2.0;
    
    for (int i = 0; i < max_iterations; i++) {
        OptionParams temp_params = params;
        temp_params.volatility = vol_mid;
        
        double calculated_price = calculatePrice(temp_params);
        double diff = calculated_price - market_price;
        
        if (std::abs(diff) < tolerance) {
            return vol_mid;
        }
        
        if (diff > 0) {
            vol_high = vol_mid;
        } else {
            vol_low = vol_mid;
        }
        
        vol_mid = (vol_low + vol_high) / 2.0;
    }
    
    return vol_mid;
}

double BlackScholes::normalCDF(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

double BlackScholes::normalPDF(double x) {
    return std::exp(-0.5 * x * x) / std::sqrt(2.0 * M_PI);
}

double BlackScholes::d1(const OptionParams& params) {
    return (std::log(params.spot_price / params.strike_price) + 
            (params.risk_free_rate + 0.5 * params.volatility * params.volatility) * params.time_to_expiry) /
           (params.volatility * std::sqrt(params.time_to_expiry));
}

double BlackScholes::d2(const OptionParams& params) {
    return d1(params) - params.volatility * std::sqrt(params.time_to_expiry);
}

} // namespace options
} // namespace hedgefund