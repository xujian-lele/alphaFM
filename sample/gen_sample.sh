
echo '' | awk '{print "1\0012\0023\0024\0025\0026"}' > df_sample
for i in {1..500000};do
  echo '' | awk '{print "1\0012\0023\0024\0025\0026"}' >> df_sample
done