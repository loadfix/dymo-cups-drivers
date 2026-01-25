#ifndef LABELMANAGER_DRIVER_LEGACY_H
#define LABELMANAGER_DRIVER_LEGACY_H

namespace DymoPrinterDriver
{



class LabelManagerDriver: public IPrinterDriver
{
public:
  typedef enum
  {
    TAPE_WIDTH_6MM = 0,
    TAPE_WIDTH_9MM,
    TAPE_WIDTH_12MM,
    TAPE_WIDTH_19MM,
    TAPE_WIDTH_24MM,
    TAPE_WIDTH_32MM
  } tape_width_t;

  typedef enum
  {
    CUT_OPTION_CUT = 0,
    CUT_OPTION_CHAIN_MARKS
  } cut_option_t;

  typedef enum
  {
    ALIGN_CENTER = 0,
    ALIGN_LEFT,
    ALIGN_RIGHT
  } alignment_t;

  typedef enum
  {
    TAPE_COLOR_BLACK_ON_WHITE = 0,
    TAPE_COLOR_BLACK_ON_BLUE,
    TAPE_COLOR_BLACK_ON_RED,
    TAPE_COLOR_BLACK_ON_SILVER,
    TAPE_COLOR_BLACK_ON_YELLOW,
    TAPE_COLOR_BLACK_ON_GOLD,
    TAPE_COLOR_BLACK_ON_GREEN,
    TAPE_COLOR_BLACK_ON_FLUORESCENT_GREEN,
    TAPE_COLOR_BLACK_ON_FLUORESCENT_RED,
    TAPE_COLOR_WHITE_ON_CLEAR,
    TAPE_COLOR_WHITE_ON_BLACK,
    TAPE_COLOR_BLUE_ON_WHITE,
    TAPE_COLOR_RED_ON_WHITE
  } tape_color_t;

  LabelManagerDriver(IPrintEnvironment& environment);
  virtual ~LabelManagerDriver();

  virtual void startDoc();
  virtual void endDoc();

  virtual void startPage();
  virtual void endPage();

  virtual void processRasterLine(const buffer_t& lineBuffer);

  void setDeviceName(const std::string& deviceName);
  void setSupportAutoCut(bool value);
  void setTSDevice(bool value);
  void setCutOptions(cut_option_t value);
  void setAlignment(alignment_t value);
  void setContinuousPaper(bool value);
  void setPrintChainMarksAtDocEnd(bool value);
  void setAutoPaper(bool value);
  void setTapeAlignmentOffset(int value);
  void setTapeColor(tape_color_t value);

  void setMaxPrintableWidth(size_t value);
  void setNormalLeader(size_t value);
  void setMinLeader(size_t value);
  void setAlignedLeader(size_t value);
  void setMinPageLines(size_t value);

  const std::string&  getDeviceName();
  bool                isSupportAutoCut();
  bool                isTSDevice();
  cut_option_t        getCutOptions();
  alignment_t         getAlignment();
  bool                isContinuousPaper();
  bool                isPrintChainMarksAtDocEnd();
  bool                isAutoPaper();
  tape_color_t        getTapeColor();
  int                 getTapeAlignmentOffset();
  size_t              getMaxPrintableWidth();
  size_t              getNormalLeader();
  size_t              getMinLeader();
  size_t              getAlignedLeader();
  size_t              getMinPageLines();

  static buffer_t getRequestStatusCommand();

protected:
  // helper function to send printer commands
  void sendCommand(const byte* buffer, size_t bufferSize);
  void sendCommand(const buffer_t& buffer);
  void sendCommandTS(const buffer_t& buffer);
  void flushCommandTS();
  void endCommandTS();
  void sendDotTab(size_t value);
  void sendCut();
  void sendChainMark();
  void sendBytesPerLine(size_t value);
  void sendSkipLines(size_t value);
  void sendTapeColor(tape_color_t value);

  void getBlanks(const buffer_t& buffer, size_t& leaderBlanks, size_t& trailerBlanks);

  size_t getMaxBytesPerLine();
private:
  IPrintEnvironment& environment;

  // job params
  cut_option_t    cutOptions;
  alignment_t     alignment;
  bool            continuousPaper;
  bool            printChainMarksAtDocEnd;
  bool            autoPaper; // don't send last empty lines
  int             tapeAlignmentOffset; // offset to justify output for the current label type . it is different for different tape sizes and models
  tape_color_t    tapeColor;

  // device params
  std::string     deviceName;
  bool            supportAutoCut;
  bool            tsDevice;
  size_t          maxPrintableWidth; // in dots
  size_t          normalLeader;
  size_t          minLeader;
  size_t          alignedLeader;
  size_t          minPageLines;

  // job internal variables
  size_t          lastDotTab;
  size_t          lastBytesPerLine;
  size_t          emptyLinesCount;
  size_t          pageNo;
  size_t          pageLineCount;

  std::vector<buffer_t> rasterLines;
  buffer_t              shiftedRasterLine;

  buffer_t tsBuffer;

  FILE*           hLockFile;
  void processRasterLineInternal(const buffer_t& lineBuffer);
  void sendCachedRasterLines();
  void shiftData(const buffer_t& buffer, buffer_t& shiftedBuffer, int shiftValue);
  void shiftDataLeft(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue);
  void shiftDataRight(const buffer_t& buffer, buffer_t& shiftedBuffer, size_t shiftValue);
  int  getShiftValue(size_t rasterLineSize);
};


}; //namespace

#endif // LABELMANAGER_DRIVER_LEGACY_H
