// Same modal/finish/release lifecycle as the Zelda settings, limited to controls.
@interface PortZeldaShortcutChoices : UITableViewController
@property(nonatomic) unsigned slot;
@property(nonatomic,strong) PortZeldaTouchState *touchState;
@end
@implementation PortZeldaShortcutChoices
- (instancetype)init {return [super initWithStyle:UITableViewStyleInsetGrouped];}
- (void)viewDidLoad {[super viewDidLoad];self.title=[NSString stringWithFormat:@"K%u",_slot+1];}
- (NSInteger)tableView:(UITableView *)table numberOfRowsInSection:(NSInteger)section {return PortShortcutChoices().count;}
- (UITableViewCell *)tableView:(UITableView *)table cellForRowAtIndexPath:(NSIndexPath *)path {
    UITableViewCell *cell=[table dequeueReusableCellWithIdentifier:@"key"]?:[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"key"];
    NSDictionary *choice=PortShortcutChoices()[path.row];cell.textLabel.text=choice[@"label"];
    cell.accessoryType=[choice[@"id"] isEqual:PortShortcut(_slot)[@"id"]]?UITableViewCellAccessoryCheckmark:UITableViewCellAccessoryNone;
    return cell;
}
- (void)tableView:(UITableView *)table didSelectRowAtIndexPath:(NSIndexPath *)path {
    releaseTouches(_touchState.pad);
    [NSUserDefaults.standardUserDefaults setObject:PortShortcutChoices()[path.row][@"id"] forKey:_slot?@"PortPadShortcut2":@"PortPadShortcut1"];
    [_touchState.pad setNeedsDisplay];[self.navigationController popViewControllerAnimated:YES];
}
@end
@interface PortZeldaControlSettings : UITableViewController <UIAdaptivePresentationControllerDelegate>
@property(nonatomic,strong) PortZeldaTouchState *touchState;
@property(nonatomic) BOOL finished;
- (void)finishSettings;
@end
@implementation PortZeldaControlSettings
- (instancetype)init {return [super initWithStyle:UITableViewStyleInsetGrouped];}
- (void)viewDidLoad {
    [super viewDidLoad];self.title=@"输入设置";
    [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(gyroCalibrationFinished:) name:@"PortGyroCalibrationFinished" object:nil];
    self.navigationItem.rightBarButtonItem=[[UIBarButtonItem alloc] initWithTitle:@"完成" style:UIBarButtonItemStyleDone target:self action:@selector(done)];
}
- (void)viewWillAppear:(BOOL)animated {[super viewWillAppear:animated];[self.tableView reloadData];}
- (NSInteger)numberOfSectionsInTableView:(UITableView *)table {return 2;}
- (NSInteger)tableView:(UITableView *)table numberOfRowsInSection:(NSInteger)section {return section==1?8:4;}
- (NSString *)tableView:(UITableView *)table titleForFooterInSection:(NSInteger)section {
    if(section==1)return @"转动 iPad 控制右摇杆视角。右拇指操作优先；设置和布局编辑期间暂停，外接手柄连接时停用。常开模式在游戏菜单中也会发送右摇杆输入。仅瞄准模式由 ZL/LT 触发。手感还受游戏内手柄灵敏度与死区影响。可单独调整横向和纵向幅度，不改变手指摇杆。校准时请放稳 iPad 约 1 秒，不要边转动边校准。游戏约 30 帧时体感视角也受此刷新限制。";
    return @"请先在游戏内暂停。按键位置、大小和透明度在编辑布局中调整。触屏鼠标已移除；外接手柄继续使用原生映射。";
}
- (UITableViewCell *)tableView:(UITableView *)table cellForRowAtIndexPath:(NSIndexPath *)path {
    if(path.section==1)return [self gyroCell:table path:path];
    UITableViewCell *cell=[table dequeueReusableCellWithIdentifier:@"control"]?:[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"control"];
    cell.accessoryView=nil;cell.accessoryType=UITableViewCellAccessoryNone;cell.detailTextLabel.text=nil;
    cell.userInteractionEnabled=YES;cell.textLabel.textColor=UIColor.labelColor;cell.selectionStyle=UITableViewCellSelectionStyleDefault;
    if(path.row==0) {
        cell.textLabel.text=@"显示虚拟手柄";UISwitch *toggle=[UISwitch new];toggle.on=controlsVisible();
        [toggle addTarget:self action:@selector(toggleControls:) forControlEvents:UIControlEventValueChanged];cell.accessoryView=toggle;cell.selectionStyle=UITableViewCellSelectionStyleNone;
    } else if(path.row==1) {
        cell.textLabel.text=@"编辑布局";cell.accessoryType=UITableViewCellAccessoryDisclosureIndicator;
        cell.userInteractionEnabled=controlsVisible()&&!gPhysicalMode;
        if(!cell.userInteractionEnabled)cell.textLabel.textColor=UIColor.tertiaryLabelColor;
    } else {
        unsigned slot=(unsigned)path.row-2;cell.textLabel.text=[NSString stringWithFormat:@"自定义 K%u",slot+1];
        cell.detailTextLabel.text=PortShortcut(slot)[@"label"];cell.accessoryType=UITableViewCellAccessoryDisclosureIndicator;
    }return cell;
}
- (void)toggleControls:(UISwitch *)toggle {
    releaseTouches(_touchState.pad);[NSUserDefaults.standardUserDefaults setBool:toggle.on forKey:BOTW_CONTROLS_VISIBLE_KEY];
    [_touchState refreshPreferences];[self.tableView reloadData];
}
- (void)tableView:(UITableView *)table didSelectRowAtIndexPath:(NSIndexPath *)path {
    if(path.section==1) {
        if(path.row==3&&PortGyroEnabled()&&!gPhysicalMode){PortGyroRecenter();[table reloadRowsAtIndexPaths:@[path] withRowAnimation:UITableViewRowAnimationNone];}
        [table deselectRowAtIndexPath:path animated:YES];return;
    }
    if(path.row==1 && controlsVisible()&&!gPhysicalMode) {
        PortZeldaTouchState *s=_touchState;
        [self dismissViewControllerAnimated:YES completion:^{[self finishSettings];[s begin];}];
    } else if(path.row>=2) {
        PortZeldaShortcutChoices *choices=[PortZeldaShortcutChoices new];choices.slot=(unsigned)path.row-2;choices.touchState=_touchState;
        [self.navigationController pushViewController:choices animated:YES];
    }
}
#include "PortGyroSettings.inc"
- (void)finishSettings {
    if(_finished)return;_finished=YES;PortGyroSuspend();_touchState.settingsPresented=NO;
    releaseTouches(_touchState.pad);[_touchState refreshPreferences];
}
- (void)done {[self dismissViewControllerAnimated:YES completion:^{[self finishSettings];}];}
- (void)presentationControllerDidDismiss:(UIPresentationController *)controller {[self finishSettings];}
@end
@implementation PortZeldaSettingsPresenter
+ (void)presentForState:(PortZeldaTouchState *)s {
    if(!s.pad.window || s.editor || s.settingsPresented)return;
    UIViewController *host=s.pad.window.rootViewController;while(host.presentedViewController)host=host.presentedViewController;
    if(!host || host.isBeingDismissed || host.isBeingPresented)return;
    s.settingsPresented=YES;PortGyroSuspend();releaseTouches(s.pad);[s placeEntry];[s.pad setNeedsDisplay];
    PortZeldaControlSettings *settings=[PortZeldaControlSettings new];settings.touchState=s;
    UINavigationController *nav=[[UINavigationController alloc] initWithRootViewController:settings];
    nav.modalPresentationStyle=UIModalPresentationFormSheet;nav.preferredContentSize=CGSizeMake(580,480);
    [host presentViewController:nav animated:YES completion:^{nav.presentationController.delegate=settings;}];
}
@end
