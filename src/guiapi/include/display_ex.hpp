/**
 * @file display_ex.hpp
 * @brief C++ wrappers over low level display api (like) st7789v so it does not need to know advanced types like font or rectangle
 */

#pragma once

#include "guitypes.hpp"
#include "Rect16.h"
#include "raster_opfn.hpp"
#include "fonts.hpp"

void display_ex_clear(const color_t clr);

void display_ex_set_pixel(point_ui16_t pt, color_t clr);

void display_ex_draw_rect(Rect16 rc, color_t clr);

void display_ex_fill_rect(Rect16 rc, color_t clr);

bool display_ex_draw_char(point_ui16_t pt, uint8_t charX, uint8_t charY, const font_t *pf, color_t clr_bg, color_t clr_fg);

void display_ex_draw_line(point_ui16_t pt0, point_ui16_t pt1, color_t clr);

color_t display_ex_get_pixel(point_ui16_t pt);

uint8_t *display_ex_get_block(point_ui16_t start, point_ui16_t end);

/*!*************************************************************************************************
 * \brief Draws rounded frames with parametric value of corner radius and corner flag (what corners it prints on).
 *       It draws into display buffer and uses SPI to send buffer to display only if buffer is full or drawing is done, which minimizes SPI overhead.
 *       ! This will fail if something else changes display buffer in the meantime !
 *
 * \param [in] rect - rounded rect coordinates (in pixels)
 *
 * \param [in] back - background color
 *
 * \param [in] front - foreground color
 *
 * \param [in] cor_rad - chosen corner radius
 *
 * \param [in] cor_flag - what corners we print (MIC_TOP_RIGHT, MIC_TOP_LEFT, MIC_BOT_RIGHT, MIC_BOT_LEFT, MIC_ALL_CORNERS)
 ***************************************************************************************************/
void display_ex_draw_rounded_rect(Rect16 rect, color_t back, color_t front, uint8_t cor_rad, uint8_t cor_flag, color_t secondary_col);

uint32_t display_ex_buffer_pixel_size();

void display_ex_store_char_in_buffer(uint16_t char_cnt, uint16_t curr_char_idx, uint8_t charX, uint8_t charY, const font_t *pf, color_t clr_bg, color_t clr_fg);

void display_ex_draw_from_buffer(point_ui16_t pt, uint16_t w, uint16_t h);

void display_ex_set_pixel_displayNativeColor(point_ui16_t pt, uint16_t noClr);

uint16_t display_ex_get_pixel_displayNativeColor(point_ui16_t pt);

/**
 * @brief draws png from config
 * not taking file directly is important, because png file can contain multiple pngs (ram optimization)
 * @param pt                    top left point
 * @param png                   png config file
 * @param back_color            background color
 * @param rop                   raster config struct
 * @param subrect               sub rectangle inside png - area to draw
 */
void display_ex_draw_png(point_ui16_t pt, const img::Resource &png, color_t back_color, ropfn rop, Rect16 subrect);
