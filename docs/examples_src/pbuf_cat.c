gsm_pbuf_p a, b;

/* Create 2 pbufs of different sizes */
a = gsm_pbuf_new(10);
b = gsm_pbuf_new(20);

/* Link them together with concat operation */
/* Reference on b will stay as is, won't be increased */
gsm_pbuf_cat(a, b);

/*
 * Operating with b variable has from now on undefined behavior,
 * application shall stop using variable b to access pbuf.
 *
 * The best way would be to set b reference to NULL
 */
b = NULL;

/*
 * When application doesn't need pbufs anymore,
 * free a and it will also free b
 */
gsm_pbuf_free(a);