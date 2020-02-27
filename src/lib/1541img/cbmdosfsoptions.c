#include <1541img/cbmdosfsoptions.h>

void CbmdosFsOptions_applyOverrides(CbmdosFsOptions *self,
        const CbmdosFsOptOverrides *overrides)
{
    uint8_t interleave = CbmdosFsOptOverrides_interleave(overrides);
    if (interleave != 0xff) self->fileInterleave = interleave;

    CbmdosFsFlags mask = overrides->mask;
    mask &= ~CFF_OVERRIDE_INTERLEAVE;
    self->flags &= ~mask;
    self->flags |= (overrides->flags & mask);
}

uint8_t CbmdosFsOptOverrides_interleave(const CbmdosFsOptOverrides *self)
{
    if (!(self->mask & CFF_OVERRIDE_INTERLEAVE)) return 0xff;
    return (self->flags & CFF_OVERRIDE_INTERLEAVE & 0xff);
}

void CbmdosFsOptOverrides_setInterleave(
        CbmdosFsOptOverrides *self, uint8_t interleave)
{
    self->flags &= ~CFF_OVERRIDE_INTERLEAVE;
    if (interleave == 0xff)
    {
        self->mask &= ~CFF_OVERRIDE_INTERLEAVE;
    }
    else
    {
        self->mask |= CFF_OVERRIDE_INTERLEAVE;
        self->flags |= (interleave & CFF_OVERRIDE_INTERLEAVE);
    }
}

