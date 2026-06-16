#include "geo_utils.h"

#include <cmath>

namespace {
constexpr double kEarthRadiusM = 6371008.8;
}

double degreesToRadians(double degrees) { return degrees * M_PI / 180.0; }

double haversineDistanceMeters(double lat1, double lon1, double lat2,
                               double lon2) {
  const double dlat = degreesToRadians(lat2 - lat1);
  const double dlon = degreesToRadians(lon2 - lon1);
  const double rlat1 = degreesToRadians(lat1);
  const double rlat2 = degreesToRadians(lat2);
  const double a = sin(dlat / 2.0) * sin(dlat / 2.0) +
                   cos(rlat1) * cos(rlat2) * sin(dlon / 2.0) *
                       sin(dlon / 2.0);
  return 2.0 * kEarthRadiusM * atan2(sqrt(a), sqrt(1.0 - a));
}
