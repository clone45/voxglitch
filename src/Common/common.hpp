float rescaleWithPadding(float t, float src_low, float src_high, float dst_low, float dst_high, float padding_at_start, float padding_at_end)
{
  return(rescale(t, src_low, src_high, (dst_low + padding_at_start),(dst_high - padding_at_end)));
}

// returns a random float between -1 and 1
// From there, you can use rescale to map to a more useful range
float randomFloat(float min, float max)
{
  float r = (float)rand() / (float)RAND_MAX;
  return(min + r * (max - min));
}

/*
float wrapFloat(float number, float max)
{
  // While this is error prone, it should work for my cases
  while(number > max) number -= max;
  while(number < 0) number += max;
}
*/
