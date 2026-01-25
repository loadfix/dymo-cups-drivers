#ifndef LABELWRITER_DRIVER_LEGACY_H
#define LABELWRITER_DRIVER_LEGACY_H

namespace DymoPrinterDriver
{



class LabelWriterDriver: public IPrinterDriver
{
public:
  typedef enum
  {
    PRINT_DENSITY_LOW = 0,
    PRINT_DENSITY_MEDIUM,
    PRINT_DENSITY_NORMAL,
    PRINT_DENSITY_HIGH
  } density_t;

  typedef enum
  {
    PRINT_QUALITY_TEXT = 0,
    PRINT_QUALITY_BARCODE_AND_GRAPHICS
  } quality_t;

  typedef enum
  {
    PAPER_TYPE_REGULAR = 0,
    PAPER_TYPE_CONTINUOUS
  } paper_type_t;

  typedef enum
  {
    RESOLUTION_UNKNOWN = 0, // unknown or irrelevant resolution
    RESOLUTION_136, // 136x204 dpi (SE450)
    RESOLUTION_204  // 204x204 dpi (SE450)
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

  LabelWriterDriver(IPrintEnvironment& environment);
  virtual ~LabelWriterDriver();

  virtual void startDoc();
  virtual void endDoc();

  virtual void startPage();
  virtual void endPage();

  virtual void processRasterLine(const buffer_t& lineBuffer);

  resolution_t getResolution();
  density_t    getDensity();
  quality_t    getQuality();
  size_t       getPageHeight();
  paper_type_t getPaperType();

  void         setResolution   (resolution_t  value);
  void         setDensity      (density_t     value);
  void         setQuality      (quality_t     value);
  void         setPageHeight   (size_t        value);
  void         setPaperType    (paper_type_t  value);
  void         setMaxPrintWidth(size_t        value);
  void         setPageOffset   (point_t       value);

  static buffer_t getResetCommand();
  static buffer_t getRequestStatusCommand();
protected:
  // helper function to send printer commands
  void sendCommand(const byte* buffer, size_t bufferSize);
  void sendCommand(const buffer_t& buffer);
  void sendLineTab(size_t value);
  void sendDotTab(size_t value);
  void sendFormFeed();
  void sendBytesPerLine(size_t value);
  void sendSkipLines(size_t value);
  void sendLabelLength(size_t value);
  void sendPrintQuality(quality_t value);
  void sendPrintDensity(density_t value);
  void sendResolution(resolution_t value);

  void getBlanks(const buffer_t& buffer, size_t& leaderBlanks, size_t& trailerBlanks);
  void sendNotCompressedData(const buffer_t& buffer, size_t leaderBlanks, size_t trailerBlanks);
  void sendCompressedData(const buffer_t& compressedBuffer, size_t notCompressedSize);

  size_t getEmptyLinesCount();
  void   setEmptyLinesCount(size_t value);



private:
  IPrintEnvironment& environment;

  resolution_t resolution;
  density_t    density;
  quality_t    quality;
  size_t       pageHeight;
  paper_type_t paperType;
  size_t       maxPrintWidth;
  point_t      pageOffset;

  size_t lastDotTab;
  size_t lastBytesPerLine;
  size_t emptyLinesCount;


};

class LabelWriterDriver400: public LabelWriterDriver
{
public:
  LabelWriterDriver400(IPrintEnvironment& environment);
  virtual ~LabelWriterDriver400();

  virtual void startDoc();
  virtual void endDoc();
  virtual void endPage();

  static buffer_t getShortFormFeedCommand();
protected:
  void sendShortFormFeed();
};

class LabelWriterDriverTwinTurbo: public LabelWriterDriver400
{
public:
  typedef enum
  {
    ROLL_AUTO = 0,
    ROLL_LEFT,
    ROLL_RIGHT
  } roll_t;

  LabelWriterDriverTwinTurbo(IPrintEnvironment& environment);
  virtual ~LabelWriterDriverTwinTurbo();

  virtual void startDoc();

  roll_t getRoll();
  void   setRoll(roll_t value);

  static buffer_t getRollSelectCommand(roll_t value);
protected:
  void sendRollSelect(roll_t value);

private:
  roll_t roll;
};

}; //namespace

#endif // LABELWRITER_DRIVER_LEGACY_H
