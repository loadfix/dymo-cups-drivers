#ifndef LABELWRITER_DRIVER_LEGACY_H
#define LABELWRITER_DRIVER_LEGACY_H

namespace DymoPrinterDriver
{



class LabelWriterDriver: public IPrinterDriver
{
public:
  typedef enum
  {
    pdLow = 0,
    pdMedium,
    pdNormal,
    pdHigh
  } density_t;

  typedef enum
  {
    pqText = 0,
    pqBarcodeAndGraphics
  } quality_t;

  typedef enum
  {
    ptRegular = 0,
    ptContinuous
  } paper_type_t;

  typedef enum
  {
    resUnknown = 0, // unknown or irrelevant resolution
    res136, // 136x204 dpi (SE450)
    res204  // 204x204 dpi (SE450)
  } resolution_t;

  struct point_t
  {
    int x;
    int y;

    point_t(int xx, int yy)
    {
      x = xx;
      y = yy;
    }
  };

  LabelWriterDriver(IPrintEnvironment& Environment);
  virtual ~LabelWriterDriver();

  virtual void StartDoc();
  virtual void EndDoc();

  virtual void StartPage();
  virtual void EndPage();

  virtual void ProcessRasterLine(const buffer_t& LineBuffer);

  resolution_t GetResolution();
  density_t    GetDensity();
  quality_t    GetQuality();
  size_t       GetPageHeight();
  paper_type_t GetPaperType();

  void         SetResolution   (resolution_t  Value);
  void         SetDensity      (density_t     Value);
  void         SetQuality      (quality_t     Value);
  void         SetPageHeight   (size_t        Value);
  void         SetPaperType    (paper_type_t  Value);
  void         SetMaxPrintWidth(size_t        Value);
  void         SetPageOffset   (point_t       Value);

  static buffer_t GetResetCommand();
  static buffer_t GetRequestStatusCommand();
protected:
  // helper function to send printer commands
  void SendCommand(const byte* Buf, size_t BufSize);
  void SendCommand(const buffer_t& Buf);
  void SendLineTab(size_t Value);
  void SendDotTab(size_t Value);
  void SendFormFeed();
  void SendBytesPerLine(size_t Value);
  void SendSkipLines(size_t Value);
  void SendLabelLength(size_t Value);
  void SendPrintQuality(quality_t Value);
  void SendPrintDensity(density_t Value);
  void SendResolution(resolution_t Value);

  void GetBlanks(const buffer_t& Buf, size_t& LeaderBlanks, size_t& TrailerBlanks);
  void SendNotCompressedData(const buffer_t& Buf, size_t LeaderBlanks, size_t TrailerBlanks);
  void SendCompressedData(const buffer_t& CompressedBuf, size_t NotCompressedSize);

  size_t GetEmptyLinesCount();
  void   SetEmptyLinesCount(size_t Value);



private:
  IPrintEnvironment& Environment_;

  resolution_t Resolution_;
  density_t    Density_;
  quality_t    Quality_;
  size_t       PageHeight_;
  paper_type_t PaperType_;
  size_t       MaxPrintWidth_;
  point_t      PageOffset_;

  size_t LastDotTab_;
  size_t LastBytesPerLine_;
  size_t EmptyLinesCount_;


};

class LabelWriterDriver400: public LabelWriterDriver
{
public:
  LabelWriterDriver400(IPrintEnvironment& Environment);
  virtual ~LabelWriterDriver400();

  virtual void StartDoc();
  virtual void EndDoc();
  virtual void EndPage();

  static buffer_t GetShortFormFeedCommand();
protected:
  void SendShortFormFeed();
};

class LabelWriterDriverTwinTurbo: public LabelWriterDriver400
{
public:
  typedef enum
  {
    rtAuto = 0,
    rtLeft,
    rtRight
  } roll_t;

  LabelWriterDriverTwinTurbo(IPrintEnvironment& Environment);
  virtual ~LabelWriterDriverTwinTurbo();

  virtual void StartDoc();

  roll_t GetRoll();
  void   SetRoll(roll_t Value);

  static buffer_t GetRollSelectCommand(roll_t Value);
protected:
  void SendRollSelect(roll_t Value);

private:
  roll_t Roll_;
};

}; //namespace

#endif // LABELWRITER_DRIVER_LEGACY_H
