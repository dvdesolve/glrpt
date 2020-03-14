/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 *
 *  http://www.gnu.org/copyleft/gpl.txt
 */


#ifndef COMMON_H
#define COMMON_H    1

#include <gtk/gtk.h>
#include <errno.h>
#include <math.h>
#include <inttypes.h>
#include <semaphore.h>
#include <pthread.h>
#include <complex.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/*---------------------------------------------------------------------------*/

/* Flow control flags */
#define STATUS_RECEIVING        0x00000001 /* SDR Receiver Running or Not     */
#define STATUS_STREAMING        0x00000002 /* SDR Receiver Streaming or Not   */
#define STATUS_DECODING         0x00000004 /* Image Decoder Running or Not    */
#define STATUS_DEMODULATING     0x00000008 /* LRPT Deomdulator Running or Not */
#define STATUS_SOAPYSDR_INIT    0x00000010 /* SoapySDR Device's Init Status   */
#define STATUS_PENDING          0x00000020 /* Wait for an Outstanding Status  */
#define STATUS_IDOQPSK_STOP     0x00000040 /* Stop decoding IDOQPSK (80k mde) */
#define STATUS_FLAGS_ALL        0x0000007F /* All Status Flags (to Clear)     */
#define ALARM_ACTION_START      0x00000080 /* Start Operation on SIGALRM      */
#define ALARM_ACTION_STOP       0x00000100 /* Stop Operation on SIGALRM       */
#define START_STOP_TIMER        0x00000200 /* Start-Stop Timer is activated   */
#define ENABLE_DECODE_TIMER     0x00000400 /* Enable Decode Operation Timer   */
#define IMAGE_RAW               0x00000800 /* Save image in raw decoded state */
#define IMAGE_NORMALIZE         0x00001000 /* Histogram normalize Wx image    */
#define IMAGE_CLAHE             0x00002000 /* CLAHE image contrast enhance    */
#define IMAGE_COLORIZE          0x00004000 /* Pseudo colorize wx image        */
#define IMAGE_COLORIZED         0x00008000 /* Pseudo colorize wx image        */
#define IMAGE_INVERT            0x00010000 /* Rotate wx image 180 degrees     */
#define IMAGES_PROCESSED        0x00020000 /* Images have been processed OK   */
#define IMAGE_RECTIFY           0x00040000 /* Rectify (stretch) wx image      */
#define IMAGES_RECTIFIED        0x00080000 /* Images Rectified OK             */
#define IMAGE_OUT_SPLIT         0x00100000 /* Save individual channel image   */
#define IMAGE_OUT_COMBO         0x00200000 /* Combine & save channel images   */
#define FRAME_OK_ICON           0x00400000 /* Decoder Frame icon showing OK   */
#define IMAGE_SAVE_JPEG         0x00800000 /* Save channel images as JPEG     */
#define IMAGE_SAVE_PPGM         0x01000000 /* Save channel image as PGM|PPM   */
#define TUNER_GAIN_AUTO         0x02000000 /* Set tuner gain to auto mode     */
#define AUTO_DETECT_SDR         0x04000000 /* Auto detect SDR device & driver */

/*---------------------------------------------------------------------------*/

/* Number of APID image channels */
#define CHANNEL_IMAGE_NUM   3

/* Indices to Normalization Range black and white values */
#define NORM_RANGE_BLACK    0
#define NORM_RANGE_WHITE    1

/* Max and Min filter bandwidth */
#define MIN_BANDWIDTH   100000
#define MAX_BANDWIDTH   200000

/* Runtime config data */
typedef struct
{
  /* glrpt working directory and glade file */
  char
    glrpt_dir[64],
    glrpt_glade[64];

  uint32_t
    invert_palette[3],  /* Image APID to invert palette (black<-->white) */
    image_scale,        /* Scale factor to fit images in glrpt live display */
    decode_timer,       /* Time duration in sec of image decoding operation */
    default_timer;      /* Default timer length in sec for image decoding   */

  char satellite_name[32]; /* Name of selected satellite */

  /* SoapySDR Configuration */
  char device_driver[32];  /* Device driver name */
  uint32_t device_index;   /* Device index */

  /* SDR Receiver configuration */
  int freq_correction; /* Freq correction factor in ppm */

  uint32_t
    sdr_center_freq, /* Center Frequency for SDR Tuner */
    sdr_samplerate,  /* SDR RX ADC sample rate */
    sdr_buf_length,  /* SDR RX ADC samples buffer length */
    sdr_filter_bw;   /* SDR Low Pass Filter Bandwidth */

  /* SDR receiver tuner gain */
  double tuner_gain;

  /* Integer FFT factors */
  uint32_t ifft_decimate; /* FFT stride (decimation) */

  /* LRPT Demodulator Parameters */
  double
    demod_samplerate,   /* SDR Rx I/Q Sample Rate S/s */
    rrc_alpha,          /* Raised Root Cosine alpha factor */
    costas_bandwidth,   /* Costas PLL Loop Bandwidth */
    pll_locked,         /* Lower phase error threshold for PLL lock */
    pll_unlocked;       /* Upper phase error threshold for PLL unlock */

  uint32_t
    rrc_order,          /* Raised Root Cosine filter order */
    symbol_rate,        /* Transmitted QPSK Symbol Rate Sy/s */
    interp_factor;      /* Demodulator Interpolation Multiplier */

  /* LRPT Decoder Parameters (channel apid) */
  uint8_t apid[CHANNEL_IMAGE_NUM];

  /* Channels to combine to produce color image */
  uint8_t color_channel[CHANNEL_IMAGE_NUM];

  /* Image Normalization pixel value ranges */
  uint8_t norm_range[CHANNEL_IMAGE_NUM][2];

  uint8_t
    psk_mode,          /* Select QPSK or OQPSK demodulator */
    rectify_function,  /* Select rectification function, either W2RG or 5B4AZ */
    clouds_threshold,  /* Pixel values above which we assumed it is cloudy areas */
    colorize_blue_max, /* Max value of blue pixels to enhance in pseudo-colorized image */
    colorize_blue_min; /* Min value of blue pixels in pseudo-colorized image */

  float jpeg_quality;  /* JPEG compressor quality factor */

} rc_data_t;

/* DSP filter data */
typedef struct
{
  double
    cutoff, /* Cutoff frequency as fraction of sample rate */
    ripple; /* Passband ripple as a percentage */

  uint32_t
    npoles, /* Number of poles, must be even */
    type;   /* Filter type as below */

  /* a and b Coefficients of the filter */
  double *a, *b;

  /* Saved input and output values */
  double *x, *y;

  /* Ring buffer index for above */
  uint32_t ring_idx;

  /* Input samples buffer and its length */
  double *samples_buf;
  uint32_t samples_buf_len;

} filter_data_t;

/* Filter type for above struct */
enum
{
  FILTER_LOWPASS = 0,
  FILTER_HIGHPASS,
  FILTER_BANDPASS
};

/* Image channels (0-2) */
enum
{
  RED = 0,
  GREEN,
  BLUE
};

/* Size of char arrays (strings) for error messages etc */
#define MESG_SIZE   128

/* Maximum time duration in sec
 * of satellite signal processing */
#define MAX_OPERATION_TIME  1000

/* Number of QPSK constellation points to plot.
 *** Must be <= SYM_CHUNKSIZE / 2 ***/
#define QPSK_CONST_POINTS   512

/*---------------------------------------------------------------------------*/

/* LRPT Demodulator data */
typedef struct
{
  double   average;
  double   gain;
  double   target_ampl;
  complex double bias;
} Agc_t;

typedef enum
{
  QPSK = 1, /* Standard QPSK as for Meteor M2 @72k sym rate */
  DOQPSK,   /* Differential Offset QPSK as for Meteor M2-2 @72k sym rate */
  IDOQPSK   /* Interleaved DOQPSK as for Meteor M2-2 @80k sym rate */
} ModScheme;

typedef struct
{
  double  nco_phase, nco_freq;
  double  alpha, beta;
  double  damping, bandwidth;
  uint8_t locked;
  double  moving_average;
  ModScheme mode;
} Costas_t;

typedef struct
{
  complex double *restrict memory;
  uint32_t fwd_count;
  uint32_t stage_no;
  double  *restrict fwd_coeff;
} Filter_t;

typedef struct
{
  Agc_t    *agc;
  Costas_t *costas;
  double    sym_period;
  uint32_t  sym_rate;
  ModScheme mode;
  Filter_t *rrc;
} Demod_t;

/*---------------------------------------------------------------------------*/

/* LRPT Decoder data */
#define PATTERN_SIZE        64
#define PATTERN_CNT         8
#define CORR_LIMIT          55
#define HARD_FRAME_LEN      1024
#define FRAME_BITS          8192
#define SOFT_FRAME_LEN      16384
#define MIN_CORRELATION     45
#define MIN_TRACEBACK       35      // 5*7
#define TRACEBACK_LENGTH    105     // 15*7
#define NUM_STATES          128

/* Number of Meteor-M LRPT image lines per second.
 * The measured value is ~6.5 but we want it as int */
#define IMAGE_LINES_PERSEC  13 / 2

/* My addition, width (pixels per line) of image (MCU_PER_LINE * 8) */
#define METEOR_IMAGE_WIDTH 1568

/* Flags to select images to output */
enum
{
  OUT_COMBO = 1,
  OUT_SPLIT,
  OUT_BOTH
};

/* Flags to indicate image file type to save as */
enum
{
  SAVEAS_JPEG = 1,
  SAVEAS_PGM,
  SAVEAS_BOTH
};

/* Flags to indicate image to be saved */
enum
{
  CH0 = 0,
  CH1,
  CH2,
  COMBO
};

/* Bit input-output data */
typedef struct
{
  uint8_t *p;
  int pos, len;
  uint8_t cur;
  int cur_len;
} bit_io_rec_t;

/* Decoder correlator data */
typedef struct
{
  uint8_t patts[PATTERN_SIZE][PATTERN_SIZE];

  int
    correlation[PATTERN_CNT],
    tmp_corr[PATTERN_CNT],
    position[PATTERN_CNT];
} corr_rec_t;

/* Decoder AC table data */
typedef struct
{
  int run, size, len;
  uint32_t mask, code;
} ac_table_rec_t;

/* Viterbi decoder data */
typedef struct
{
  int BER;

  uint16_t dist_table[4][65536];
  uint8_t  table[NUM_STATES];
  uint16_t distances[4];

  bit_io_rec_t bit_writer;

  //pair_lookup
  uint32_t pair_keys[64];      //1 shl (order-1)
  uint32_t *pair_distances;
  size_t   pair_distances_len; // My addition, size of above pointer's alloc
  uint32_t pair_outputs[16];   //1 shl (2*rate)
  uint32_t pair_outputs_len;

  uint8_t history[MIN_TRACEBACK + TRACEBACK_LENGTH][NUM_STATES];
  uint8_t fetched[MIN_TRACEBACK + TRACEBACK_LENGTH];
  int hist_index, len, renormalize_counter;

  int err_index;
  uint16_t errors[2][NUM_STATES];
  uint16_t *read_errors, *write_errors;
} viterbi27_rec_t;

/* Decoder MTD data */
typedef struct
{
  corr_rec_t c;
  viterbi27_rec_t v;

  int pos, prev_pos;
  uint8_t ecced_data[HARD_FRAME_LEN];

  uint32_t word, cpos, corr, last_sync;
  int r[4], sig_q;
} mtd_rec_t;

/*-------------------------------------------------------------------*/

// JPEG chroma subsampling factors. Y_ONLY (grayscale images)
// and H2V2 (color images) are the most common.
enum subsampling_t
{
  Y_ONLY = 0,
  H1V1   = 1,
  H2V1   = 2,
  H2V2   = 3
};

// JPEG compression parameters structure.
typedef struct
{
  // Quality: 1-100, higher is better. Typical values are around 50-95.
  float m_quality;

  // m_subsampling:
  // 0 = Y (grayscale) only
  // 1 = YCbCr, no subsampling (H1V1, YCbCr 1x1x1, 3 blocks per MCU)
  // 2 = YCbCr, H2V1 subsampling (YCbCr 2x1x1, 4 blocks per MCU)
  // 3 = YCbCr, H2V2 subsampling (YCbCr 4x1x1, 6 blocks per MCU-- very common)
  enum subsampling_t m_subsampling;

  // Disables CbCr discrimination - only intended for testing.
  // If true, the Y quantization table is also used for the CbCr channels.
  gboolean m_no_chroma_discrim_flag;

} compression_params_t;

/*-------------------------------------------------------------------*/

/* DSP Filter Parameters */
#define FILTER_RIPPLE   5.0
#define FILTER_POLES    6

/* Return values */
#define ERROR       1
#define SUCCESS     0

/* Generel definitions for image processing */
#define MAX_FILE_NAME   128 /* Max length for filenames */

/* Should have been in math.h */
#ifndef M_2PI
  #define M_2PI     6.28318530717958647692
#endif

#define BYTE_IMAGE  1

#ifdef BYTE_IMAGE /* for 8 bit-per-pixel images */
  typedef unsigned char kz_pixel_t;
  #define uiNR_OF_GREY ( 256 )
#else /* for 12 bit-per-pixel images (default) */
  typedef unsigned short kz_pixel_t;
  #define uiNR_OF_GREY ( 4096 )
#endif

/*---------------------------------------------------------------------*/

/* Function prototypes produced by cproto */
/* callback_func.c */
void Error_Dialog(void);
gboolean Cancel_Timer(gpointer data);
void Popup_Menu(void);
void Start_Togglebutton_Toggled(GtkToggleButton *togglebutton);
void Start_Receiver_Menuitem_Toggled(GtkCheckMenuItem *menuitem);
void Decode_Images_Menuitem_Toggled(GtkCheckMenuItem *menuitem);
void Alarm_Action(void);
void Decode_Timer_Setup(void);
void Auto_Timer_OK_Clicked(void);
void Hours_Entry(GtkEditable *editable);
void Minutes_Entry(GtkEditable *editable);
void Enter_Center_Freq(uint32_t freq);
void Fft_Drawingarea_Size_Alloc(GtkAllocation *allocation);
void Qpsk_Drawingarea_Size_Alloc(GtkAllocation *allocation);
void Qpsk_Drawingarea_Draw(cairo_t *cr);
void BW_Entry_Activate(GtkEntry *entry);
void Set_Check_Menu_Item(gchar *item_name, gboolean flag);
/* callbacks.c */
void on_main_window_destroy(GObject *object, gpointer user_data);
gboolean on_main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
gboolean on_image_viewport_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void on_start_receiver_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_decode_images_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_save_images_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_decode_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_decode_timer_dialog_destroy(GObject *object, gpointer user_data);
void on_decode_timer_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_auto_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_auto_timer_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_auto_timer_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_auto_timer_hrs_changed(GtkEditable *editable, gpointer user_data);
void on_auto_timer_min_changed(GtkEditable *editable, gpointer user_data);
void on_auto_times_activate(GtkEntry *entry, gpointer user_data);
void on_auto_stop_min_activate(GtkEntry *entry, gpointer user_data);
void on_cancel_timer_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_auto_timer_destroy(GObject *object, gpointer user_data);
void on_satellite_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
void on_raw_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_normalize_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_clahe_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_rectify_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_invert_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_combine_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_individual_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_pseudo_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_jpeg_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_pgm_menuitem_toggled(GtkCheckMenuItem *menuitem, gpointer user_data);
void on_quit_menuitem_activate(GtkMenuItem *menuitem, gpointer user_data);
gboolean on_error_dialog_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_quit_dialog_destroy(GObject *object, gpointer user_data);
void on_error_dialog_destroy(GObject *object, gpointer user_data);
void on_error_ok_button_clicked(GtkButton *button, gpointer user_data);
void on_error_quit_button_clicked(GtkButton *button, gpointer user_data);
void on_quit_cancel_button_clicked(GtkButton *button, gpointer user_data);
void on_quit_button_clicked(GtkButton *button, gpointer user_data);
gboolean on_ifft_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_qpsk_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_sdr_bw_entry_activate(GtkEntry *entry, gpointer user_data);
void on_sdr_freq_entry_activate(GtkEntry *entry, gpointer user_data);
gboolean on_sig_level_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_sig_qual_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_agc_gain_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean on_pll_ave_drawingarea_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data);
void on_sdr_gain_scale_value_changed(GtkRange *range, gpointer user_data);
void on_start_togglebutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_manual_agc_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_auto_agc_radiobutton_toggled(GtkToggleButton *togglebutton, gpointer user_data);
/* clahe.c */
gboolean CLAHE(kz_pixel_t *pImage, uint32_t uiXRes, uint32_t uiYRes, kz_pixel_t Min, kz_pixel_t Max, uint32_t uiNrX, uint32_t uiNrY, uint32_t uiNrBins, double fCliplimit);
/* display.c */
void Display_Waterfall(void);
void Display_QPSK_Const(int8_t *buffer);
void Display_Icon(GtkWidget *img, const gchar *name);
void Display_Demod_Params(Demod_t *demod);
void Draw_Level_Gauge(GtkWidget *widget, cairo_t *cr, double level);
/* image.c */
void Normalize_Image(uint8_t *image_buffer, uint32_t image_size, uint8_t range_low, uint8_t range_high);
void Flip_Image(uint8_t *image_buffer, uint32_t image_size);
void Display_Scaled_Image(uint8_t *chan_image[], uint32_t apid, int current_y);
void Create_Combo_Image(uint8_t *combo_image);
/* interface.c */
GtkWidget *Builder_Get_Object(GtkBuilder *builder, gchar *name);
GtkWidget *create_main_window(GtkBuilder **builder);
GtkWidget *create_popup_menu(GtkBuilder **builder);
GtkWidget *create_timer_dialog(GtkBuilder **builder);
GtkWidget *create_error_dialog(GtkBuilder **builder);
GtkWidget *create_startstop_timer(GtkBuilder **builder);
GtkWidget *create_quit_dialog(GtkBuilder **builder);
/* jpeg.c */
gboolean jepg_encoder_compression_parameters(compression_params_t *comp_params, float m_quality, enum subsampling_t m_subsampling, gboolean m_no_chroma_discrim_flag);
gboolean jepg_encoder_compress_image_to_file(char *file_name, int width, int height, int num_channels, const uint8_t *pImage_data, compression_params_t *comp_params);
/* main.c */
int main(int argc, char *argv[]);
void Initialize_Top_Window(void);
/* rc_config.c */
gboolean Load_Config(gpointer data);
gboolean Find_Config_Files(gpointer data);
/* utils.c */
void File_Name(char *file_name, uint32_t chn, const char *ext);
void Usage(void);
void Show_Message(const char *mesg, const char *attr);
void mem_alloc(void **ptr, size_t req);
void mem_realloc(void **ptr, size_t req);
void free_ptr(void **ptr);
gboolean Open_File(FILE **fp, char *fname, const char *mode);
void Save_Image_JPEG(char *file_name, int width, int height, int num_channels, const uint8_t *pImage_data, compression_params_t *comp_params);
void Save_Image_Raw(char *fname, const char *type, uint32_t width, uint32_t height, uint32_t max_val, uint8_t *buffer);
void Cleanup(void);
int isFlagSet(int flag);
int isFlagClear(int flag);
void SetFlag(int flag);
void ClearFlag(int flag);
void ToggleFlag(int flag);
void Strlcpy(char *dest, const char *src, size_t n);
void Strlcat(char *dest, const char *src, size_t n);
double dClamp(double x, double min, double max);
int iClamp(int i, int min, int max);
/* alib.c */
int Count_Bits(uint32_t n);
uint32_t Bio_Peek_n_Bits(bit_io_rec_t *b, const int n);
void Bio_Advance_n_Bits(bit_io_rec_t *b, const int n);
uint32_t Bio_Fetch_n_Bits(bit_io_rec_t *b, const int n);
void Bit_Writer_Create(bit_io_rec_t *w, uint8_t *bytes, int len);
void Bio_Write_Bitlist_Reversed(bit_io_rec_t *w, uint8_t *l, int len);
int Ecc_Decode(uint8_t *idata, int pad);
void Ecc_Deinterleave(uint8_t *data, uint8_t *output, int pos, int n);
void Ecc_Interleave(uint8_t *data, uint8_t *output, int pos, int n);
/* correlator.c */
int Hard_Correlate(const uint8_t d, const uint8_t w);
void Init_Correlator_Tables(void);
void Fix_Packet(void *data, int len, int shift);
void Correlator_Init(corr_rec_t *c, uint64_t q);
int Corr_Correlate(corr_rec_t *c, uint8_t *data, uint32_t len);
/* dct.c */
void Flt_Idct_8x8(double *res, const double *inpt);
/* huffman.c */
int Get_AC(const uint16_t w);
int Get_DC(const uint16_t w);
int Map_Range(const int cat, const int vl);
void Default_Huffman_Table(void);
/* medet.c */
void Medet_Init(void);
void Medet_Deinit(void);
void Decode_Image(uint8_t *in_buffer, int buf_len);
double Sig_Quality(void);
/* met_jpg.c */
void Mj_Dump_Image(void);
void Mj_Dec_Mcus(uint8_t *p, uint32_t apid, int pck_cnt, int mcu_id, uint8_t q);
void Mj_Init(void);
/* met_packet.c */
void Parse_Cvcdu(uint8_t *p, int len);
/* met_to_data.c */
void Mtd_Init(mtd_rec_t *mtd);
uint8_t **ret_decoded(void);
gboolean Mtd_One_Frame(mtd_rec_t *mtd, uint8_t *raw);
/* rectify_meteor.c */
void Rectify_Images(void);
/* viterbi27.c */
double Vit_Get_Percent_BER(const viterbi27_rec_t *v);
void Vit_Decode(viterbi27_rec_t *v, uint8_t *input, uint8_t *output);
void Mk_Viterbi27(viterbi27_rec_t *v);
/* agc.c */
Agc_t *Agc_Init(void);
_Complex double Agc_Apply(Agc_t *self, _Complex double sample);
void Agc_Free(Agc_t *self);
/* demod.c */
void Demod_Init(void);
void Demod_Deinit(void);
double Agc_Gain(double *gain);
double Signal_Level(uint32_t *level);
double Pll_Average(void);
gboolean Demodulator_Run(gpointer data);
/* doqpsk.c */
void De_Interleave(uint8_t *raw, int raw_siz, uint8_t **resync, int *resync_siz);
void Make_Isqrt_Table(void);
void Free_Isqrt_Table(void);
void De_Diffcode(int8_t *buff, uint32_t length);
/* filters.c */
Filter_t *Filter_New(uint32_t fwd_count, double *fwd_coeff);
Filter_t *Filter_RRC(uint32_t order, uint32_t factor, double osf, double alpha);
_Complex double Filter_Fwd(Filter_t *const self, _Complex double in);
void Filter_Free(Filter_t *self);
/* pll.c */
Costas_t *Costas_Init(double bw, ModScheme mode);
_Complex double Costas_Mix(Costas_t *self, _Complex double samp);
void Costas_Correct_Phase(Costas_t *self, double error);
void Costas_Recompute_Coeffs(Costas_t *self, double damping, double bw);
void Costas_Free(Costas_t *self);
double Costas_Delta(_Complex double sample, _Complex double cosample);
/* utils.c */
int8_t Clamp_Int8(double x);
double Clamp_Double(double x, double max_abs);
/* SoapySDR.c */
gboolean SoapySDR_Set_Center_Freq(uint32_t center_freq);
void SoapySDR_Set_Tuner_Gain_Mode(void);
void SoapySDR_Set_Tuner_Gain(double gain);
gboolean SoapySDR_Init(void);
gboolean SoapySDR_Activate_Stream(void);
void SoapySDR_Close_Device(void);
/* filters.c */
gboolean Init_Chebyshev_Filter(filter_data_t *filter_data, uint32_t buf_len, uint32_t filter_bw, double sample_rate, double ripple, uint32_t num_poles, uint32_t type);
void Enter_Filter_BW(void);
void DSP_Filter(filter_data_t *filter_data);
void Deinit_Chebyshev_Filter(filter_data_t *data);
/* ifft.c */
gboolean Initialize_IFFT(int16_t width);
void Deinit_Ifft(void);
void IFFT(int16_t *data);
void IFFT_Real(int16_t *data);
void IFFT_Data(short sample);

#endif

