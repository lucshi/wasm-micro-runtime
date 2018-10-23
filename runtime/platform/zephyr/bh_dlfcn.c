#include <zephyr.h>
#include <kernel.h>
#include <misc/printk.h>
#include <gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*symbol_func)(void);

struct NativeSymbol {
  const char *symbol;
  symbol_func func_ptr;
} NativeSymbol;

#define REG_SYMBOL(symbol) {#symbol, (symbol_func)symbol}

static int
putchar(int c)
{
  printk("%c", (char)c);
  return 1;
}

static int
gpio_pin_configure_wrapper(struct device *port, u32_t pin, int flags)
{
  return gpio_pin_configure(port, pin, flags);
}

static int
gpio_pin_read_wrapper(struct device *port, u32_t pin, u32_t *value)
{
  return gpio_pin_read(port, pin, value);
}

static int
gpio_pin_write_wrapper(struct device *port, u32_t pin, u32_t value)
{
  return gpio_pin_write(port, pin, value);
}

static const struct NativeSymbol native_symbol_defs[] = {
  REG_SYMBOL(device_get_binding),
  { "gpio_pin_configure", (symbol_func)gpio_pin_configure_wrapper },
  { "gpio_pin_read", (symbol_func)gpio_pin_read_wrapper },
  { "gpio_pin_write", (symbol_func)gpio_pin_write_wrapper },
  REG_SYMBOL(memcmp),
  REG_SYMBOL(memcpy),
  REG_SYMBOL(memmove),
  REG_SYMBOL(memset),
  REG_SYMBOL(putchar),
  REG_SYMBOL(snprintf),
  REG_SYMBOL(sprintf),
  REG_SYMBOL(strchr),
  REG_SYMBOL(strcmp),
  REG_SYMBOL(strcpy),
  REG_SYMBOL(strlen),
  REG_SYMBOL(strncmp),
  REG_SYMBOL(strncpy),
};

void *
bh_dlsym(void *handle, const char *symbol)
{
  int low = 0;
  int high = sizeof(native_symbol_defs) / sizeof(NativeSymbol) - 1;
  int mid, ret;

  if (!symbol)
    return NULL;

  while (low <= high) {
    mid = (low + high) / 2;
    ret = strcmp(symbol, native_symbol_defs[mid].symbol);

    if (ret == 0)
      return native_symbol_defs[mid].func_ptr;
    else if (ret < 0)
      high = mid - 1;
    else
      low = mid + 1;
  }

  return NULL;
}

