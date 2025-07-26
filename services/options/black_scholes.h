#pragma once

#include <cmath>

namespace hedgefund {
namespace options {

struct OptionParams {
    double spot_price;      // Current stock price
    double strike_price;    // Strike price
    double time_to_expiry;  // Time to expiration (in years)
    double risk_free_rate;  // Risk-free interest rate
    double volatility;      // Implied volatility
    bool is_call;          // true for call, false for put
};

struct Greeks {
    double delta;    // Price sensitivity to underlying
    double gamma;    // Delta sensitivity to underlying
    double theta;    // Time decay
    double vega;     // Volatility sensitivity
    double rho;      // Interest rate sensitivity
};

class BlackScholes {
public:
    static double calculatePrice(const OptionParams& params);
    static Greeks calculateGreeks(const OptionParams& params);
    
    static double impliedVolatility(double market_price, const OptionParams& params, 
                                   double tolerance = 1e-6, int max_iterations = 100);
    
private:
    static double normalCDF(double x);
    static double normalPDF(double x);
    static double d1(const OptionParams& params);
    static double d2(const OptionParams& params);
};

} // namespace options
} // namespace hedgefund