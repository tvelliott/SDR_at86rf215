/* This file was automatically generated.  Do not edit! */
#ifndef __EQLMS_RF_H__
#define __EQLMS_RF_H__

struct eqlms_rf_s {
    unsigned int h_len;     // filter length
    float        mu;        // LMS step size

    // internal matrices
    float *h0;        // initial coefficients
    float *w0;        // weights [px1]
    float *w1;        // weights [px1]

    unsigned int count;     // input sample count
    int          buf_full;  // input buffer full flag
    struct window_rf_s *buffer;    // input buffer
    struct wdelay_rf_s *x2;        // buffer of |x|^2 values
    float        x2_sum;    // sum{ |x|^2 }
};

void eqlms_rf_train(struct eqlms_rf_s *_q,float *_w,float *_x,float *_d,unsigned int _n);
void eqlms_rf_get_weights(struct eqlms_rf_s *_q,float *_w);
void eqlms_rf_step(struct eqlms_rf_s *_q,float _d,float _d_hat);
void eqlms_rf_step_blind(struct eqlms_rf_s *_q,float _d_hat);
void eqlms_rf_execute_block(struct eqlms_rf_s *_q,unsigned int _k,float *_x,unsigned int _n,float *_y);
void eqlms_rf_execute(struct eqlms_rf_s *_q,float *_y);
void eqlms_rf_push_block(struct eqlms_rf_s *_q,float *_x,unsigned int _n);
void eqlms_rf_update_sumsq(struct eqlms_rf_s *_q,float _x);
void eqlms_rf_push(struct eqlms_rf_s *_q,float _x);
void eqlms_rf_set_bw(struct eqlms_rf_s *_q,float _mu);
float eqlms_rf_get_bw(struct eqlms_rf_s *_q);
struct eqlms_rf_s *eqlms_rf_create_lowpass(unsigned int _h_len,float _fc);
struct eqlms_rf_s *eqlms_rf_create_rnyquist(int _type,unsigned int _k,unsigned int _m,float _beta,float _dt);
void eqlms_rf_reset(struct eqlms_rf_s *_q);
struct eqlms_rf_s *eqlms_rf_create(float *_h,unsigned int _h_len);

#endif
