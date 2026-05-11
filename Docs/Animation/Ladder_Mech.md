# **Архитектурное руководство по интеграции механики вертикальных лестниц в 2.5D-платформерах на Unreal Engine 5.6**

Проектирование и реализация механики взаимодействия игрового персонажа с вертикальной лестницей в рамках 2.5D-платформера требует решения комплекса физических, математических и сетевых задач.1 В отличие от стандартных трехмерных игр, где перемещение персонажа свободно во всех направлениях, 2.5D-перспектива накладывает жесткие ограничения на плоскость движения, требуя при этом сохранения глубины трехмерного окружения и бесшовной стыковки анимаций.3 Данный отчет представляет собой детальное техническое руководство по созданию надежной системы лазания по лестницам на C++ для Unreal Engine 5.6 с поддержкой клиентского предсказания движения (Client-Side Prediction), процедурного выравнивания и динамической фильтрации коллизий.1

## ---

**Системные требования и особенности 2.5D-перспективы**

Разработка платформера с трехмерным миром и видом сбоку (2.5D) накладывает специфические ограничения на взаимодействие персонажа с лестницами.4 Персонаж перемещается по фиксированной игровой плоскости (обычно XZ), а его визуальное представление в обычном режиме ориентировано в профиль к камере (разворот по оси Yaw на ![][image1] или ![][image2]).5 При переходе в режим лазания персонаж должен ориентироваться лицом к лестнице (спиной к камере, разворот Yaw на ![][image3] или ![][image4] в зависимости от ориентации лестницы в пространстве).6

Основными проблемами при реализации данной механики являются:

* **Дрейф по глубине:** Смещение физической капсулы персонажа относительно центральной оси лестницы по неигровой оси (Y), что приводит к визуальному несоответствию или падению персонажа.5  
* **Двойное поведение верхней площадки:** Необходимость динамического переключения роли верхней перекладины лестницы с проходимого препятствия на твердый опорный пол.3  
* **Мгновенное изменение ориентации меша:** Необходимость плавной интерполяции угла поворота меша без нарушения траектории физического движения капсулы.9

## ---

**Конечно-автоматная модель жизненного цикла лазания**

Логика взаимодействия персонажа с лестницей наиболее эффективно описывается в виде конечного автомата, интегрированного в кастомный компонент движения.6 Жизненный цикл перемещения состоит из трех основных фаз: LadderStart, LadderLoop и LadderEnd.1

                     \[ Нажатие "Вверх" / Overlap \]  
    Character Idle/Walk \------------------------------\> LadderStart

| |  
| | \[Конец анимации\]  
| v  
| \<----------------------------------------- LadderLoop \<---- Character Jump/Fall (Overlap в воздухе)  
| \[Достижение низа лестницы\] |  
| |  
| | \[Достижение верха лестницы\]  
| v  
    Character Idle/Walk \<------------------------------- LadderEnd  
                             \[Конец анимации выхода\]

### **Вход на лестницу и фазы инициализации**

Переход в состояние лазания зависит от текущего физического состояния персонажа.1

* **Обычный заход (Character Idle/Walk ![][image5] LadderStart):** Игрок управляет персонажем на земле.1 При входе в триггерную зону коллизии лестницы и нажатии клавиши направления («Вверх» или «Вниз») запускается переходное состояние.1 Включается фаза LadderStart, блокирующая стандартное управление перемещением.18 Воспроизводится переходная анимация монтажа (AnimMontage), а физическая капсула персонажа плавно смещается (интерполируется) к целевой точке входа на лестницу.1 По окончании воспроизведения анимации конечный автомат автоматически переходит в состояние LadderLoop.1  
* **Заход из прыжка или падения (Character Jump/Fall ![][image5] LadderLoop):** Персонаж находится в воздухе и пересекает коллизию ствола лестницы.1 При удержании или нажатии клавиши ввода («Вверх» или «Вниз») персонаж мгновенно захватывает лестницу.2 Проигрывание начальной анимации перехода (LadderStart) полностью пропускается во избежание визуальных задержек и разрыва динамики.1 Система мгновенно переходит в режим циклического движения LadderLoop.1

### **Циклическое движение и обработка особых случаев**

Состояние LadderLoop описывает непрерывное вертикальное перемещение персонажа.1 Направление движения вверх и вниз определяется знаком проекции вектора ввода на вертикальную ось лестницы.1 При отсутствии ввода персонаж фиксируется на текущей высоте, сила гравитации отключается.15

В таблице ниже систематизированы переходы между базовыми состояниями, включая неупомянутые краевые кейсы взаимодействия:

| Исходное состояние | Триггер перехода | Целевое состояние | Физический режим UE | Описание поведения и анимации |
| :---- | :---- | :---- | :---- | :---- |
| **Idle / Walk** | Overlap триггера низа \+ Ввод «Вверх» 1 | **LadderStart** 1 | MOVE\_Custom 6 | Запуск монтажа входа, плавное выравнивание капсулы по оси лестницы.1 |
| **Jump / Fall** | Overlap триггера ствола \+ Любой вертикальный ввод 2 | **LadderLoop** 1 | MOVE\_Custom 6 | Мгновенный захват лестницы без проигрывания анимации входа.1 |
| **LadderLoop** | Достижение нижней границы лестницы 1 | **Idle / Walk** 1 | MOVE\_Walking 19 | Бесшовный переход на землю, восстановление гравитации и управления.1 |
| **LadderLoop** | Достижение верхней границы лестницы при движении вверх 1 | **LadderEnd** 1 | MOVE\_Custom 6 | Запуск монтажа взбирания на уступ верхней площадки.1 |
| **Idle / Walk (Верх)** | Нажатие «Вниз» над верхней зоной лестницы 15 | **LadderEnd (Реверс)** | MOVE\_Custom 6 | Проигрывание анимации перешагивания через край уступа вниз.10 |
| **LadderLoop** | Нажатие клавиши прыжка во время лазания 1 | **Jump / Fall** | MOVE\_Falling 14 | Прыжок от лестницы. Активируется кулдаун повторного захвата.2 |
| **LadderLoop** | Получение урона / Эффект отбрасывания (Knockback) | **Jump / Fall** | MOVE\_Falling 14 | Нарушение захвата. Персонаж срывается с лестницы и переходит в режим падения. |
| **LadderLoop** | Нажатие клавиши ускорения (Slide Down) | **LadderLoop (Slide)** | MOVE\_Custom 6 | Персонаж быстро скользит вниз по лестнице без перешагивания по перекладинам. |

## ---

**Архитектура кастомного компонента движения на C++**

Использование стандартного режима полета (MOVE\_Flying) для реализации лазания по лестницам является распространенной архитектурной ошибкой.19 Режим полета не учитывает специфику взаимодействия с вертикальными поверхностями, требует постоянной ручной корректировки сил трения и торможения, а также делает невозможным использование встроенного предсказания перемещения для сетевой игры.19

Наиболее надежное решение — создание кастомного режима физики в рамках наследуемого класса от UCharacterMovementComponent (CMC).6

Для этого в заголовочном файле класса кастомного CMC объявляется перечисление режимов движения 1:

C++

\#**pragma** once

\#**include** "CoreMinimal.h"  
\#**include** "GameFramework/CharacterMovementComponent.h"  
\#**include** "MyCharacterMovementComponent.generated.h"

UENUM(BlueprintType)  
enum class ECustomMovementMode : uint8  
{  
    CM\_None      UMETA(DisplayName \= "None"),  
    CM\_Climbing  UMETA(DisplayName \= "Climbing")  
};

UCLASS()  
class YOURPROJECT\_API UMyCharacterMovementComponent : public UCharacterMovementComponent  
{  
    GENERATED\_BODY()

public:  
    UMyCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category \= "Movement | Climbing")  
    float MaxClimbSpeed \= 250.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category \= "Movement | Climbing")  
    float ClimbingDeceleration \= 1000.f;

    bool IsClimbing() const;

    void StartClimbing();  
    void StopClimbing();

protected:  
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override; \[6\]  
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode) override; \[6\]  
    virtual float GetMaxSpeed() const override; \[2\]  
    virtual float GetMaxBrakingDeceleration() const override; \[2\]

    void PhysClimbLadder(float DeltaTime, int32 Iterations); \[1, 2\]

public:  
    uint8 bWantsToClimb : 1;  
};

### **Реализация физического режима в PhysCustom**

Метод PhysCustom является центральной точкой обработки всех кастомных режимов перемещения персонажа.6 При переключении физики в режим MOVE\_Custom выполнение стандартных алгоритмов приостанавливается, и управление передается кастомной расчетной функции 6:

C++

\#**include** "MyCharacterMovementComponent.h"  
\#**include** "GameFramework/Character.h"

UMyCharacterMovementComponent::UMyCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)  
    : Super(ObjectInitializer)  
{  
    bWantsToClimb \= false;  
}

bool UMyCharacterMovementComponent::IsClimbing() const  
{  
    return (MovementMode \== MOVE\_Custom && CustomMovementMode \== static\_cast\<uint8\>(ECustomMovementMode::CM\_Climbing));  
}

void UMyCharacterMovementComponent::StartClimbing()  
{  
    SetMovementMode(MOVE\_Custom, static\_cast\<uint8\>(ECustomMovementMode::CM\_Climbing)); \[2, 6\]  
}

void UMyCharacterMovementComponent::StopClimbing()  
{  
    SetMovementMode(MOVE\_Walking); \[15, 19\]  
}

void UMyCharacterMovementComponent::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)  
{  
    Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

    if (IsClimbing())  
    {  
        // Сброс текущей скорости и отключение гравитации при входе на лестницу  
        Velocity \= FVector::ZeroVector;  
        SetZeroGravity();  
    }  
    else if (PrevMovementMode \== MOVE\_Custom && PrevCustomMode \== static\_cast\<uint8\>(ECustomMovementMode::CM\_Climbing))  
    {  
        // Восстановление стандартного режима гравитации при выходе  
        bWantsToClimb \= false;  
    }  
}

void UMyCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)  
{  
    if (CustomMovementMode \== static\_cast\<uint8\>(ECustomMovementMode::CM\_Climbing))  
    {  
        PhysClimbLadder(DeltaTime, Iterations);  
    }  
      
    Super::PhysCustom(DeltaTime, Iterations); \[6\]  
}

void UMyCharacterMovementComponent::PhysClimbLadder(float DeltaTime, int32 Iterations)  
{  
    if (DeltaTime \< MIN\_TICK\_TIME)  
    {  
        return;  
    }

    // Очистка аддитивных сил корневого движения перед расчетом кастомной скорости   
    RestorePreAdditiveRootMotionVelocity(); \[2\]

    // Извлечение вектора ввода от контроллера  
    FVector InputVector \= GetPendingMovementInputVector();  
    float VerticalInput \= InputVector.Z\!= 0.0f? InputVector.Z : InputVector.X; // Адаптация ввода 2.5D-сцены \[15, 16\]

    // Расчет целевой скорости движения вдоль вертикальной оси лестницы \[1, 11\]  
    FVector ClimbDirection \= FVector::UpVector;  
    Velocity \= ClimbDirection \* (VerticalInput \* MaxClimbSpeed); \[1, 11\]

    // Расчет дельты перемещения  
    FVector Delta \= Velocity \* DeltaTime; \[11\]  
    FHitResult Hit(1.f);

    // Выполнение безопасного перемещения с обработкой коллизий  
    SafeMoveUpdatedComponent(Delta, UpdatedComponent-\>GetComponentQuat(), true, Hit);

    if (Hit.IsValidBlockingHit())  
    {  
        // Исключение застревания при столкновении с препятствиями по ходу движения  
        SlideAlongGrid(Delta, 1.f \- Hit.Time, Hit.Normal, Hit);  
    }

    // Очистка вектора ввода для следующего кадра  
    ConsumeInputVector();  
}

float UMyCharacterMovementComponent::GetMaxSpeed() const  
{  
    return IsClimbing()? MaxClimbSpeed : Super::GetMaxSpeed(); \[2\]  
}

float UMyCharacterMovementComponent::GetMaxBrakingDeceleration() const  
{  
    return IsClimbing()? ClimbingDeceleration : Super::GetMaxBrakingDeceleration(); \[2\]  
}

### **Математический алгоритм выравнивания и проецирования**

Для нивелирования дрейфа по глубине игрового пространства (ось Y) физическая капсула персонажа должна быть жестко зафиксирована на центральной оси лестницы в момент начала лазания.5

Пусть ![][image6] — мировая координата основания лестницы, а ![][image7] — мировая координата ее вершины.11 Тогда направляющий вектор лестницы ![][image8] вычисляется следующим образом 11:

![][image9]  
Для проецирования текущего положения персонажа ![][image10] на отрезок лестницы применяется формула скалярного проецирования с ограничением границ 11:

![][image11]  
Спроецированная точка привязки капсулы персонажа ![][image12] вычисляется как 11:

![][image13]  
В коде C++ этот алгоритм интегрируется в функцию активации лазания, фиксируя координату по неигровой оси Y 5:

C++

void UMyCharacterMovementComponent::SnapToLadderAxis(const FVector& LadderBottom, const FVector& LadderTop)  
{  
    FVector CurrentLocation \= UpdatedComponent-\>GetComponentLocation();  
    FVector LadderSegment \= LadderTop \- LadderBottom;  
    float SegmentLengthSq \= LadderSegment.SizeSquared();

    if (SegmentLengthSq \> 0.0f)  
    {  
        float ProjectionParam \= FVector::DotProduct(CurrentLocation \- LadderBottom, LadderSegment) / SegmentLengthSq;  
        ProjectionParam \= FMath::Clamp(ProjectionParam, 0.0f, 1.0f);

        FVector TargetSnappedLocation \= LadderBottom \+ ProjectionParam \* LadderSegment;  
          
        // В 2.5D платформере принудительно фиксируется координата глубины сцены (ось Y) \[5\]  
        TargetSnappedLocation.Y \= LadderBottom.Y;

        FHitResult SweepHit;  
        UpdatedComponent-\>SetWorldLocation(TargetSnappedLocation, false, \&SweepHit, ETeleportType::TeleportPhysics); \[7\]  
    }  
}

## ---

**Обработка двунаправленной коллизии на верхней платформе**

Верхняя площадка лестницы должна сочетать в себе два противоположных коллизионных поведения 3: она обязана быть проходимой снизу вверх при подъеме персонажа и обеспечивать надежную твердую опору для бега, когда персонаж находится сверху.3

### **Архитектурное решение с использованием маркеров позиционирования**

Для реализации этой механики в структуру класса лестницы (ALadderActor) интегрируются четыре компонента трансформации (Scene Components), жестко определяющие точки входа и выхода персонажа 1:

                     (Мир ходьбы сверху)  
|  
                      (Верхний хват)  
|  
| \<--- Ствол лестницы (Ladder Axis)  
|  
                    (Нижний хват)  
|  
                    (Точка схода на землю)

Координаты этих маркеров настраиваются дизайнерами уровней в зависимости от высоты уступа и масштаба лестницы 1:

* BottomClimbingSpot (Локальное смещение: ![][image14]) 1  
* BottomLandingSpot (Локальное смещение: ![][image15]) 1  
* TopClimbingSpot (Локальное смещение: ![][image16]) 1  
* TopLandingSpot (Локальное смещение: ![][image17]) 1

### **Алгоритм прохода сквозь платформу сверху вниз**

Когда персонаж стоит на верхней платформе и инициирует спуск по лестнице нажатием кнопки «Вниз», система выполняет следующий упорядоченный алгоритм действий 15:

1. **Детекция намерения:** Персонаж находится в триггере TopCollisionBox лестницы.1 При получении ввода «Вниз» движение MOVE\_Walking блокируется.15  
2. **Динамическая фильтрация коллизий:** Для физической капсулы персонажа временно переопределяется тип взаимодействия с поверхностью платформы (канал ECC\_WorldStatic переводится в режим Ignore или добавляется исключение столкновения с конкретным Actor платформы через IgnoreActorWhenMoving).3  
3. **Воспроизведение монтажа спуска:** Запускается анимация LadderEnd в реверсивном режиме, имитирующая аккуратное перешагивание персонажем края уступа.10  
4. **Линейная интерполяция положения:** Положение капсулы персонажа плавно интерполируется из точки TopLandingSpot в точку TopClimbingSpot.1  
5. **Фиксация и восстановление коллизий:** Как только капсула оказывается ниже уровня платформы (достигает TopClimbingSpot), коллизия с платформой восстанавливается в режим Block, а физический режим переключается на MOVE\_Custom (Climbing) в фазе LadderLoop.1

## ---

**Сетевая репликация движения и интеграция Saved Moves**

Специфика функционирования UCharacterMovementComponent в мультиплеере заключается в том, что симуляция движения выполняется независимо на стороне автономного клиента, после чего результаты отправляются на сервер для верификации и синхронизации с simulated-прокси других игроков.7 Если клиент совершает кастомное движение, о котором сервер не имеет детальной информации по вводу, сервер сочтет такое перемещение ошибкой и вернет клиента в исходную точку (произойдет визуальный откат).22

Для предотвращения этого в C++ необходимо расширить системные структуры FSavedMove\_Character и FNetworkPredictionData\_Client\_Character.6

### **Реализация расширенного класса Saved Move**

Структура FSavedMove\_Character хранит снапшот состояния персонажа и ввода на каждом тике движения.7 Нам необходимо добавить флаг намерения совершить подъем 22:

C++

class FSavedMove\_MyCharacter : public FSavedMove\_Character  
{  
public:  
    typedef FSavedMove\_Character Super;

    uint8 bSavedWantsToClimb : 1;

    virtual void Clear() override  
    {  
        Super::Clear();  
        bSavedWantsToClimb \= false;  
    }

    // Запись актуального состояния компонента движения в структуру сохранения  
    virtual void SetMoveFor(ACharacter\* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData\_Client\_Character& ClientData) override  
    {  
        Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

        if (UMyCharacterMovementComponent\* MoveComp \= Cast\<UMyCharacterMovementComponent\>(Character-\>GetCharacterMovement()))  
        {  
            bSavedWantsToClimb \= MoveComp-\>bWantsToClimb;  
        }  
    }

    // Определение возможности склейки (комбинации) двух идущих подряд ходов  
    virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter\* InCharacter, float MaxDelta) const override  
    {  
        if (bSavedWantsToClimb\!= ((FSavedMove\_MyCharacter\*)NewMove.Get())-\>bSavedWantsToClimb)  
        {  
            return false;  
        }  
        return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);  
    }

    // Сжатие пользовательских флагов в байт для отправки по сети на сервер   
    virtual uint8 GetCompressedFlags() const override  
    {  
        uint8 Result \= Super::GetCompressedFlags();

        if (bSavedWantsToClimb)  
        {  
            Result |= FLAG\_Custom\_0; // Использование выделенного бита флага   
        }

        return Result;  
    }  
};

### **Реализация сетевого предсказания на сервере**

На стороне сервера CMC переопределяет метод обработки полученных сжатых флагов, извлекая состояние намерения клиента начать лазание 7:

C++

void UMyCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)  
{  
    Super::UpdateFromCompressedFlags(Flags);

    // Извлечение бита флага и присвоение переменной на сервере  
    bWantsToClimb \= ((Flags & FLAG\_Custom\_0)\!= 0);  
}

Данная реализация гарантирует полное отсутствие сетевого рассинхронизатора даже при пинге более 100 мс и потере пакетов до 15%.2

## ---

**Анализ существующих OpenSource решений**

В игровой индустрии существует несколько ключевых открытых решений, которые могут послужить архитектурным базисом для разработки кастомной системы лазания.

### **Сравнительный анализ OpenSource-проектов для Unreal Engine**

| Проект / Репозиторий | Метод детекции лестницы | Интеграция с физикой CMC | Поддержка сетевой репликации | Особенности визуализации и IK |
| :---- | :---- | :---- | :---- | :---- |
| **ue4-climb2d-sample (Nauja)** 2 | Перекрытие триггеров объема ASampleClimbableVolume 2 | Полное расширение CMC на C++ с кастомным режимом 2 | Да, реализовано клиентское предсказание (Saved Moves) 2 | Минималистичная, ориентирована на 2D-игры без использования IK.2 |
| **parkour-climbing-ue5-cpp (Rahul Chandra)** 26 | Процедурная трассировка окружения (Line/Sphere Trace) 26 | Модификация физики на основе уклона поверхностей 26 | Требует ручной доработки репликации состояния виса | Высокоточная визуализация с интеграцией Control Rig и Motion Warping.26 |
| **LdrCharacterMovementComponent (by mdhananjay)** 1 | Комбинированное перекрытие триггерных коробок начала уступов 1 | Наследование от CMC, явный расчет векторов 1 | Да, оптимизировано под мультиплеерные сессии 1 | Базовая интерполяция к опорным точкам (Scene Components).1 |

### **Интеграция визуализации и процедурной анимации (IK)**

Проблема разворота персонажа спиной к камере во время лазания решается путем полного разделения физического вращения капсулы и визуального вращения скелетного меша.9 Физическая капсула сохраняет ориентацию вдоль плоскости платформера для корректной обработки боковых столкновений, в то время как меш плавно интерполируется лицом к лестнице.13

Для достижения максимального реализма во время фазы LadderLoop применяется процедурный анимационный контроль рук и ног 26:

1. **Двухэтапная трассировка:** На каждом кадре из костей ладоней и ступней персонажа по направлению к лестнице пускаются короткие лучи (Line Traces) для поиска точек соприкосновения с перекладинами.26  
2. **Control Rig и FABRIK:** Полученные точки контакта передаются в кастомный AnimBlueprint в качестве целевых векторов эффекторов для алгоритма FABRIK (Forward And Backward Reaching Inverse Kinematics).26 Это позволяет конечностям персонажа гарантированно наступать на перекладины лестницы независимо от ее высоты и шага ступеней, исключая эффект скольжения рук по воздуху.26

## ---

**Рекомендации по оптимизации и расширению механики**

При интеграции описанного кодового решения на платформе Unreal Engine 5.6 разработчику рекомендуется уделить внимание следующим аспектам оптимизации:

* **Оптимизация трассировки:** Для снижения нагрузки на процессор (CPU) трассировку лучей для поиска лестниц и расчета IK следует выполнять не каждый кадр, а с интервалом (например, 10-15 Гц), плавно интерполируя полученные векторы эффекторов во времени с помощью функции VInterpTo.  
* **Использование Motion Warping:** Для реализации бесшовного выхода персонажа на верхнюю площадку (LadderEnd) рекомендуется задействовать плагин Motion Warping, встроенный в UE 5.6.26 Он автоматически деформирует корневое движение анимации взбирания так, чтобы персонаж гарантированно завершил подъем ровно в точке TopLandingSpot независимо от динамически изменяющейся высоты уступа.1  
* **Перспективная миграция на фреймворк Mover:** Поскольку Epic Games позиционирует плагин Mover (Mover 2.0) как будущий стандарт для замены устаревшего CMC в последующих версиях движка, архитектуру переключаемых состояний лазания рекомендуется проектировать модульно.21 Оптимальным решением будет вынос расчетной логики скоростей и сил в отдельные чистые статические функции-хелперы, которые впоследствии можно будет перенести из PhysClimbLadder в класс режима UBaseMovementMode без полной перегрузки кодовой базы проекта.1

#### **Источники**

1. Climbing Up\! Ladders in Multiplayer | Epic Developer Community, дата последнего обращения: мая 10, 2026, [https://dev.epicgames.com/community/learning/tutorials/MP04/unreal-engine-climbing-up-ladders-in-multiplayer](https://dev.epicgames.com/community/learning/tutorials/MP04/unreal-engine-climbing-up-ladders-in-multiplayer)  
2. Nauja/ue4-climb2d-sample: Sample of a custom climbing movement done in Unreal Engine 5 with Paper2D \- GitHub, дата последнего обращения: мая 10, 2026, [https://github.com/Nauja/ue4-climb2d-sample](https://github.com/Nauja/ue4-climb2d-sample)  
3. The Ultimate Unreal Engine 2D Game Development Course from Udemy \- OpenCourser, дата последнего обращения: мая 10, 2026, [https://opencourser.com/course/qwlxj6/the-ultimate-unreal-engine-2d-game-development-course](https://opencourser.com/course/qwlxj6/the-ultimate-unreal-engine-2d-game-development-course)  
4. Blog \- Giraffe Cat, дата последнего обращения: мая 10, 2026, [http://giraffe.cat/blog](http://giraffe.cat/blog)  
5. How can I make my character move up and down (e.g. up a ladder) without using flying movement mode? : r/unrealengine \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/acxbdc/how\_can\_i\_make\_my\_character\_move\_up\_and\_down\_eg/](https://www.reddit.com/r/unrealengine/comments/acxbdc/how_can_i_make_my_character_move_up_and_down_eg/)  
6. Custom Movement Mode Creation \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/custom-movement-mode-creation/309530](https://forums.unrealengine.com/t/custom-movement-mode-creation/309530)  
7. Character Movement Component | Unreal Engine 4.27 Documentation | Epic Developer Community, дата последнего обращения: мая 10, 2026, [https://dev.epicgames.com/documentation/unreal-engine/character-movement-component?application\_version=4.27](https://dev.epicgames.com/documentation/unreal-engine/character-movement-component?application_version=4.27)  
8. In Unity, how do I implement a pathfinding AI in a 2D platformer game? \- Quora, дата последнего обращения: мая 10, 2026, [https://www.quora.com/In-Unity-how-do-I-implement-a-pathfinding-AI-in-a-2D-platformer-game](https://www.quora.com/In-Unity-how-do-I-implement-a-pathfinding-AI-in-a-2D-platformer-game)  
9. How to snap the player character left and right for a platformer without gradually rotating them? : r/unrealengine \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/9q28d6/how\_to\_snap\_the\_player\_character\_left\_and\_right/](https://www.reddit.com/r/unrealengine/comments/9q28d6/how_to_snap_the_player_character_left_and_right/)  
10. Trouble with Ladders \- Blueprint \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/trouble-with-ladders/1508336](https://forums.unrealengine.com/t/trouble-with-ladders/1508336)  
11. Custom Movement mode in Character Component : r/unrealengine \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/vvtsvy/custom\_movement\_mode\_in\_character\_component/](https://www.reddit.com/r/unrealengine/comments/vvtsvy/custom_movement_mode_in_character_component/)  
12. What's the best way to make a one-way platform for a 2.5D platformer in UE5? \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/1fjutou/whats\_the\_best\_way\_to\_make\_a\_oneway\_platform\_for/](https://www.reddit.com/r/unrealengine/comments/1fjutou/whats_the_best_way_to_make_a_oneway_platform_for/)  
13. How to rotate character fully with orient rotation to movement \- Blueprint, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/how-to-rotate-character-fully-with-orient-rotation-to-movement/1590194](https://forums.unrealengine.com/t/how-to-rotate-character-fully-with-orient-rotation-to-movement/1590194)  
14. Recreating the Climbing System from Zelda BOTW in Unreal (C++ ..., дата последнего обращения: мая 10, 2026, [https://www.vitorcantao.com/post/climbing-system/](https://www.vitorcantao.com/post/climbing-system/)  
15. How to make a ladder with Blueprint? \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/how-to-make-a-ladder-with-blueprint/277995](https://forums.unrealengine.com/t/how-to-make-a-ladder-with-blueprint/277995)  
16. How to Climb a Ladder in Unreal Engine 5 (First Person Character) | by Shaun Fulton, дата последнего обращения: мая 10, 2026, [https://medium.com/@fulton\_shaun/how-to-climb-a-ladder-in-unreal-engine-5-first-person-character-08afdde5af55](https://medium.com/@fulton_shaun/how-to-climb-a-ladder-in-unreal-engine-5-first-person-character-08afdde5af55)  
17. Unreal Engine 5 Ladder System Tutorial (First Person Setup) \- YouTube, дата последнего обращения: мая 10, 2026, [https://www.youtube.com/watch?v=a5aEvhI5sWA](https://www.youtube.com/watch?v=a5aEvhI5sWA)  
18. Simple ladder climb (no animation) \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/simple-ladder-climb-no-animation/370956](https://forums.unrealengine.com/t/simple-ladder-climb-no-animation/370956)  
19. How can I make my character move up a ladder? \- Epic Developer Community Forums, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/how-can-i-make-my-character-move-up-a-ladder/338218](https://forums.unrealengine.com/t/how-can-i-make-my-character-move-up-a-ladder/338218)  
20. Issues with climbing ladders in a 2D platformer \- Epic Developer Community Forums, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/issues-with-climbing-ladders-in-a-2d-platformer/2335665](https://forums.unrealengine.com/t/issues-with-climbing-ladders-in-a-2d-platformer/2335665)  
21. What is the equivalent to setting the movement mode to "flying"? \- Blueprint, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/what-is-the-equivalent-to-setting-the-movement-mode-to-flying/2279265](https://forums.unrealengine.com/t/what-is-the-equivalent-to-setting-the-movement-mode-to-flying/2279265)  
22. Authoritative Networked Character Movement \- wiki.unrealengine.com, дата последнего обращения: мая 10, 2026, [https://michaeljcole.github.io/wiki.unrealengine.com/Authoritative\_Networked\_Character\_Movement/](https://michaeljcole.github.io/wiki.unrealengine.com/Authoritative_Networked_Character_Movement/)  
23. Character Movement Replication : r/unrealengine \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/1d1fje8/character\_movement\_replication/](https://www.reddit.com/r/unrealengine/comments/1d1fje8/character_movement_replication/)  
24. Extending Saved Move Data \- C++ \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/extending-saved-move-data/211852](https://forums.unrealengine.com/t/extending-saved-move-data/211852)  
25. Character Movement Replication in UE4 \- error454, дата последнего обращения: мая 10, 2026, [https://error454.com/2015/03/20/character-movement-replication-in-ue4/](https://error454.com/2015/03/20/character-movement-replication-in-ue4/)  
26. Climbing Mechanics in Unreal Engine C++ \- Rahul Chandra, дата последнего обращения: мая 10, 2026, [https://www.rahulchandraportfolio.com/post/climbing-mechanics-in-unreal-engine-c](https://www.rahulchandraportfolio.com/post/climbing-mechanics-in-unreal-engine-c)  
27. Unreal Engine 5 C++: Climbing System Course Overview \- YouTube, дата последнего обращения: мая 10, 2026, [https://www.youtube.com/watch?v=jkrRBJ5azKo](https://www.youtube.com/watch?v=jkrRBJ5azKo)  
28. Procedural Parkour and Traversal Animation Techniques \- Bournemouth University, дата последнего обращения: мая 10, 2026, [https://nccastaff.bournemouth.ac.uk/jmacey/MastersProject/MSc24/02/ProceduralParkourandTraversalAnimationTechniques.pdf](https://nccastaff.bournemouth.ac.uk/jmacey/MastersProject/MSc24/02/ProceduralParkourandTraversalAnimationTechniques.pdf)  
29. Character rotation \- C++ \- Epic Developer Community Forums \- Unreal Engine, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/character-rotation/2253997](https://forums.unrealengine.com/t/character-rotation/2253997)  
30. Ladder Climbing System on UE5 \- Tutorial \- YouTube, дата последнего обращения: мая 10, 2026, [https://www.youtube.com/watch?v=IZjAgJzA654](https://www.youtube.com/watch?v=IZjAgJzA654)  
31. C++ Overriding Character Movement Component Events \- Programming & Scripting, дата последнего обращения: мая 10, 2026, [https://forums.unrealengine.com/t/c-overriding-character-movement-component-events/2391918](https://forums.unrealengine.com/t/c-overriding-character-movement-component-events/2391918)  
32. How to replace the CMC with Mover 2.0 / Character Motion Component in 5.4 Release? : r/unrealengine \- Reddit, дата последнего обращения: мая 10, 2026, [https://www.reddit.com/r/unrealengine/comments/1cbd5u4/how\_to\_replace\_the\_cmc\_with\_mover\_20\_character/](https://www.reddit.com/r/unrealengine/comments/1cbd5u4/how_to_replace_the_cmc_with_mover_20_character/)

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABsAAAAXCAYAAAD6FjQuAAABRUlEQVR4Xu2UvUoDQRSFrxIEERJBxNJWiyBEfAF9AhsrwUIE8QVS6QvYWIrkHcRWwUIQRBT8t1IQewsFtVLP2bkjN5eZhZSB/eCQOd/szO7CZkQq+pkh5AT5RQ7dnGUTeUc+kVU3R3aRM+QLmXZzBS0JN2lon9PueUCOTL9DTk1fRkZM3zLjf7jxdcLdml5X56Eb1THfyDNly7iEBXtWgkv1kSvXI3QdHa8hNTO3YcYFKxIW7Dh/rD7Cce5m1h8g+8gTMm98waSk3+xF/Zh2v2kk57Pw4puEY/ix2O7J+SwLEhbw8ydtCR8H3aC63KY5X8qEhP/XPdJEnqV7k9ymOd8T3ODH9A91HrpHL8tIPR37oulL6jx0s16WwQU8fiIXyJvpEV63bvq2up6YkbDoVX95RqYYljB/LuHE+UYGuq6oqOgL/gC84V9iDl/IsgAAAABJRU5ErkJggg==>

[image2]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAXCAYAAACMLIalAAABoElEQVR4Xu2VvyuGURTHv6QUFgOLSMiMxUBZDDbFQNlsMhhtShaDlPyIQQZlMpgUmZTCIv4Do5LFr5Jf5/ue5+o8x3173ndQhudT397nfM+597nd99z7ADk5f0e/6F70JToXVabTBZhbEE2KJkTjorFEzaauQ3QBrT8xfqBO9CE6El253A+rog0Tv0AnbDNeS+IVE/NkIIkDXS4mNy4+dHEBDuqNeHayadGyqFvUKWqHLnpWtGXqOGbKxOQNuvuBIfNM7lyMWvxeAPEet9pTBd3VQCN0DH8tx4kfODPPsbgA+6TPeX5RMd5dPIf4mB2k/QbRo2hNdGv8TDjJpzcN7MF55x0gvqh1xP2yYCNykhqfMMRecoq4vwL1m3yiVNjwnIBbXIwlxF++h7jPv4k+e7Bs6qGDq33CwZpLb6J4T20j7mfCy9IP3HVxgHWL3oQeFuayTl/JxJqat65nGPqCGZ9IYG7EeU+iB+dlwsuNk8Xk2YT6/NTE4H1mr4oKaH2r8TLhifALCXo1dYFRaK7HJwzXomfRPrR2MJ3Oycn5n3wDScF1cXUnNXcAAAAASUVORK5CYII=>

[image3]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABEAAAAYCAYAAAAcYhYyAAAA7klEQVR4XmNgGAXEAHl0ASQgAsTpQMyDLgEDJ4D4KhC7AfFjID6AIsvAUAHE+lC2ExAHI8mBwVwg/osm9h+IS5H4O5DYIHADjY+hAQRmQMVhYCMSGwQuIXOkGSCKPZEFgSAHKg4DnQyQMAEBJSBOQZJjSGCAKDZFFgSCCKi4KpKYJhB3ALEckhgYVDJAFMMCDQZAAYfNhVhBFQN2Q4Kg4uFo4lhBGgNEsQGaeAhU3BlNHCuwY4AotkQTj4WKgwKeIGBngCgOQxOHeZNoAFI8CU1sG1ScaIDNVhAfFLgkgeUMkKQPokEGFKBKj4KhCQDAsC+u1RNrMAAAAABJRU5ErkJggg==>

[image4]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACUAAAAXCAYAAACMLIalAAABpklEQVR4Xu2VzysFURTHD8mGJTY2r9hYKHtKycJSwl7ZIKLs/MgfYKNYykYsKFaK2IhiQ8RGNmIjpYSNiPPtnJk5c95M5i2UxXzq29zv99x75943c+cR5eT8PZOsIR8a5ljfrC/WdLwU0sg6Jem372qgmmT8Luvc1ULWWR8kk0DD8XLIAavD+CnWi/GgnWSOgBbnwaXzO84XkbaoctajD0n61zvvf2ls+MT4LtMGSfPGSFtUNxXvGCAraLtOPa6WPc0Djk07yReRtqgqih5vg2ZN6gNmnQ9YoXhey3plLbLuTJ5K2qLAFkULO2O9xcu0rTXPEiXnmcHgER8arihaGNRsaoeaeRZIcvvulQQGj/pQeWf1afuGooXhiIM19R48JuQVvpAVDB7zIbPKOnJZG0n/a/Vp79QyJeeZweBxH5LkwQtumaHohq3a/u30lQwGT/iQJO/1ITPIujUe/XqMBzgQzy7LTA3JpPO+wPRT8m6RVRqPv45P48tI+hRMlokN1hPrgXWvV3xl8SW2DJDcADvHjdHGRjwXJIdik6RPZ7yck5PzP/kBWQRz9AqWWSEAAAAASUVORK5CYII=>

[image5]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABMAAAAXCAYAAADpwXTaAAAAVUlEQVR4XmNgGAWjgKpgL7oAJeAfugAlwAaIy9AFKQHngNgcXRAETMjEt4B4HwMa8CMTX4NiFgYKwUQg9kYXJAcoAnEnuiC54BO6ACXgMLrAKBhuAACnlhESw2iRqwAAAABJRU5ErkJggg==>

[image6]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADsAAAAYCAYAAABEHYUrAAAB0ElEQVR4Xu2Wuy9EQRTGj1dBQkMiEo8IoZAoPBKNSuh0EpWOBo3QolOjUehUQkEhEq9C5Q+QiI5EdITC+/2dzMzuubNzb3bZFSP3l3zZ85g9d769926GKCYmJuYPMwd9pqlR/R3vmaCkqR6rNyh6h1bPS6LMMvIOe08mZtutnndkYrbA6nlHlNli0TsPtvxEmn2GHkVutJlY7TnS7ALUBDVClXKRxQ6p9d4hzU5bvSi+a3bdLpC7lhN+02wXpRpz1XLGT8zuQUvQO1QqelxbhlYoOXOWgv8DzSE1pgi6gyahB6hQ1826FmgNeoPKoSNoCzrR61JoJXVC4g2ZITyAa21iXRj2nTX5OKmZhm2oX8dszr6LrlrYbBOP6bjb0SsTeQI+G19Dl9CFFsdX0KJYF4ZrQxX6s1fUeWP3OnYZs2vV5J6dL2JDJ/Qqcu7ViDxruDZUpT8HRH2K1OPGzEAbOh4JqfFmXbNLRGzgJ/BJ5NyrFXnWkBfNE/kQtC96Z1CfjtnMgY7ZZFhNzuZT24fIZa8DehE59+pEnjXmoVVol9TrwIYNw6R+cf5zaRB15lQrndoxBQ80/Drwa3ZL6hG+IXVtvg4fiDjmWr35QkzMP+ELOMeqhb/YC64AAAAASUVORK5CYII=>

[image7]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACQAAAAYCAYAAACSuF9OAAABX0lEQVR4XmNgGAWjYBiCNiD+TyTOguqhCyhkQFjsjCYXjiR3EE2OZgCfg0AAOaToAkhxkDGaHE0AKQ5iRpOjCcDnIE4kuQeoUrQDyA76CcTfkfgwvAGumg4A2UETgVgNiFWAWBxZET0BsoNq0eTwgZPoAtQC5DoIpJ4mgBwHIaev50jih4G4F4h3A3EgVGwnA0RdDxA/A+JHQLweKocC9BggJfEiBoThq6BiRkjqcAH0EFoDxElI/LdALANl7wPieiS5P0BcgcQHA1BdBtL0FIgfQzGI/QaIJyGpwwXQHQTisyLxVwPxDigbFEpFSHIdDJj6KQYwA8uQ+CJQNghsAeJjUDbIYcgOqgbi30h8qgCYg45C6bkMqGnwLxBLQNkgByGHOijKLJH4VAGdDJDS2wVJDJQM3gPxNyDmQxIHOaibAZKoPwGxFZLcgIBdDKhRNqBACYjvMkASuSia3OAFAEytd7D5+44qAAAAAElFTkSuQmCC>

[image8]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAwAAAAZCAYAAAAFbs/PAAAAW0lEQVR4XmNgGAUjB8wE4v9IGAa6cYiDgSAOCV4c4gzMuCQYcIvjlMAljlMClzhOCVziWCUccIiDAUxCEcoXQxID4WVAbACVAwM+IL6NpGA1VBxZUydUbBTgBQDyxyyITYNUTgAAAABJRU5ErkJggg==>

[image9]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAmwAAAAwCAYAAACsRiaAAAADWUlEQVR4Xu3dPagcVRgG4KMBESVREUElSCAqklLt/KliUsU2RRAEg7ZWGrROoVgKYmNnFSTRTkgiRKJY2BgwdrGy0RD/8D/q+ZhzvOfOndm7K7syG58HPubMN3P2ztzmvuzZO5sSAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACrdyDXX9vUr7lurRMm7KO09dr79cQ/ZwMArJk21LS+HOlPWb3eHU3vlqZ/vOkDAKyNscB2XdO/s3dsqoYCW3i5OQYAsHbGAluo/Wf6ByZqLLA9XfpD9wgAMGooKA31Vm3sZ94z0p+yscD2VnMMAGAhQyHij4Fe37lcl+aok3XCDPUa6s+MpdA2rH1b+uugXnMNbLfleqnp31D6AABza4NSFQGp31ulNrDdn+u+XHdtOmP19uR6eI7aTr2PI6m7l925btx0Ruf11J0XwRQAYKaraWs4uzLQW6U2sM3jRL+xBA/lenKO2k69j8f6Bwb8nhYLbJ/0GwDA/8NvaWtQ+mWg1/dsrlfnqOfrhBmmENiWZZHAFs+YWySw9X8/L/T2AYBr1FdpcxC4uez3w8EqLRLY4jNh75ZtiM+K/VjGB1P34f5Ygvwwbbzei7kul/GqLRLYIhhfLOO47jfK+GyuR8r4z7KN+43Xrfe9M3X3GPvxnLcwNO+m1M17r+zH+NNmDACsgTtS94f759T9wY93fd4vvajXNk5durFvOvisPWlA+w7b17kON/s1hMRzz2pIafurMvZNB7NEYHuw2a/nt/OO5fp4oB8+6O3PMy/G1w/0AYA1cG+u28s4Piy/L9eujcOT8k4zjtDRfuVTDSERWP7LwPZvDAW2R8u2eq7Z79/DrMA2Nm9sDACwVGea8VO5TpdxLI/WpcAIbHXpry4LTk0EtkNlfHfaWNptr/VS6t6JbPuPl+3bZXu+bLebN2sMALBUX5Sqjua6kOtU04vAFku7Ee5+aPpTEsvQD6Tus2if947F/f2Ua2/TeyV136/aiv39zX7Mi99Fnfdmrm9yfZ+6QBjj71L32jFep2fcAQDXmHhYbbskCgDAhMQ/HMQ7R/E8OQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABgcv4GzFwD2ka0pHQAAAAASUVORK5CYII=>

[image10]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACoAAAAYCAYAAACMcW/9AAABjElEQVR4Xu2VzStEYRTGj6ysLBUlIcnCggX5KLJS9pb2FsrSgqaUsiRbJbEhwl9A+QdsFAsfyc5X8v39PHPPNee+pmluXfdS91e/5txz3pl55va+c0VSUlIiYwp+FumwvidRRiUXqM+ZDZrZjjOLnUJBib2ziRImaKszi5UwQUudWawUClpmZifBUfzYoM/w0Vz7bnyvThAbdAY2wHpYYRcVQTk8FO9zfgUbdNyZhaVa/knQKvmDQbfhMlyCI9rjduHnbME1eKx90g034TS8NH2eC76nH+7BjJllaRbvybMouaAr2msx6/KxAFe17oW7WvtBfd5gndYZOKE1f5h90r3ASdgIO00/C5/1/GXn8ExlfQFnzbp8MEyX25SfQe9hm7luguvwCh6Z/pN4hzhyGKbHbYJKCQa9he1a80ZwO5ABCW4LBq0x15HBvcy/Ip99fXVP/R3s0Jr9Eq3n4Ck80OtXWKt15IzBd/G+kI9W3s1r8bYTD8iN1uxx3/Ewfeh6hnqAQ/rqr5uXlBSPL0wPhs9X6LT/AAAAAElFTkSuQmCC>

[image11]: <data:image/png;base64,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>

[image12]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAAAYCAYAAABKtPtEAAACNElEQVR4Xu2XzUsVURjGX21lGlFQ4l5aFAp9QYW5aV/LFv0H5cY/QMSNGyWoTeSmqF20MBdC1CaoqE0QREEEJVkkWkpk35TPw3lP95nD3JsXC+o4P3iY836cmTPvfc/cGbOKioqKdc4o9HOVOulzsmTQajd6JIkdl9itJJYNjQpAtBOypJkC7E1iWdBMATYksSxoVIA2ib0ohvJBC/AF+iR21OSv7AzRApyBdkDdUKcm/aect3BfG9OAogUYSmI5UBXA/kIB+GZ4DpqAfrgvnmMXdAX6Bm31GKHNeW+hPvcdtDDnJTQLTUOvPUbiOR9CV6FlqEXinDfsOVvE/wa6DD3zWGkBei286V2y2oW4cPr2SF4ZzI3ckTH9p3x82MICI1x8ROf3WzF2GpoXW3OPin3Pwo9HOqDvPn4P7fQxqVsAfgvw13hlYaEUxwvQWckr45qFE/OiLGREF7sf+ix2KzQOPbZiHruAi45ssmJcx2rzyK6Ja1e/UrcAa2G7H+O3QkTH7KJYAN5k3CpE8w5YsQAsVL1zqs3jNg04ZfntiW/N6EX0I0n9+6CvPn4AXfAx3yaZ1+U2C6Cdwr1+XWw95wD0zsd8nkxJjJ1FPkC7xc/5m8X+I/Dhwq3yBBpzH/cxfUsW2n/Rwhb76HG2610L7xoXoafujx0wAz3yuMIbuA/dhm4msRMWOmvOQudEnkM3XJxPcWv9kxyy4hZISVs6K/hwGrHwF9mTxMgxCwX43b9SRbOsADuuwTXeAm+EAAAAAElFTkSuQmCC>

[image13]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAmwAAAAxCAYAAABnGvUlAAAEiElEQVR4Xu3d26ttUxgA8EGUlJDcI97kQfEgt1wKT1JKUZIXRSHJX+BBuTwi4UFELuWe8KJEyeXBvVxPiELu91uMz5jDHmc0115r7bPPOXud9fvV1xrzm2OsNefcu+bXmHPNlRIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAwEZyZo5/psTvOfaqA/hPf4zG4oz/ewMArIOj00qh0TquyR/arVt2j6XxY3bdhPwyOTgt9/4DwFYxqWALNf9nv2LJTSrYQs3f3K9YQGP7N81DaW3jAIBVzFKwja1bZrMUbJ/2KxbQ2P5NE2OiaAMA1tEsBdur/YolN0vBdla/YgGN7d8kdb/bWIvb0sr4T4bcjU3uuyEHAEulL9h2SuWetUeH3I5ygjw/x6YZY5qxgm3vHJ+P5BfZvPtxeZp/zJh6DGvBFvYYct83OQBYGm3B1kbct3ZE028juibHe31yG2gLtjaimDi36RfuzPFRl1sU8xZfX6XZxjzcJzpjBVvNK9gAWEr9DNui2R7bPTbDtppZ+1WXdcsvdctbw9kjEdvd506rA0ZE/yv7ZOeSVPpd2K9o/J0UbACwmWUp2A7PccOMMc22LtjmHb9e5v3c6L9zn1yDP1J5r/6LG5H7scsBwFKYp2A7MccDOd7PcU+OA1IZt2eOp3N8uNI1fZlK4RF9q3gQb/R/PZUT77VDvn7+E6nMqjwy5EM8HuOuHF83uVdSucfugzTbdq+3tRRsr+W4JZXLuNWtOe7IcXeOXYZcfd/63v1ymDQujvdbOd5M5V7EeH2x6TOvWfevavu/0LTn9Usq7/VZkzt1yP3a5ABghzfplw7eaDt1vsnx+NDeb3iNMfGQ3RAn02OH9u7Da4jCqmpP6n+lUgD2+bgE+EUqReB9Q+70HC+nst1RIFbzFhVbqj9eNVbTrn8yx4ND+7wm3/aZNsM2aVwcr6o95m17Hv3nThP942+zpV9U2TetHNdnU/m/iuJ+1uMNAEsvHvERJ8yrh+Voxwk2/JBWircoHqLvKWnzm+7bk23MEtV7ktr8BcNyXBqLXw+I4i9it1Qe+9BahJN3u40xYxj7fFLa/Cesos8hQ/uKJh/a8auNixm8qi3e2pnPecx7bONvFPe5rZf4WbR9muUjc+yfY9cmBwB02stR9WQer/WkGgXb8UM7ZuOqTTluH9ptERCPwbh0JB+zbven8vyt55p8zK6dkOPAJjdvUbE9tNsYRez1Q/uqJt/2qflzhte6ri5PGndT024LtrV+S/XjPgEAbHw/p3ISj0d+RNF0UCr3ln2byiXS2o573d5O5R6qp1K5J+miVESBEZc84z3idyeryN+bStEXRVl1WCp932lyUcQ9n+PdVMZt9KItCq247y62+eQmf3GO31I5Pq24/6/d3yjw2uWxcXHcokiOWbafhvYzQ5/4u6x1lg0AWEKTiqtJeQAAtqGjUinMjunyMesU+fqFBQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB2LP8CME1rQGbZa7gAAAAASUVORK5CYII=>

[image14]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAG8AAAAXCAYAAAAFtBHMAAAD/UlEQVR4Xu2YW8hOWRjHH+dTo3HMjBmfi0FRSOQ8yrjhwnFwgdw5pJALIkm4oLhwvpiQkrOiUMQFLhwjkpILEheEhshpzHj+31rr69n/vd7v3e/n8/a9zf7Vv/dd/7X22muvtddaz9oiOTk5OTk5/z/aqM6q/lNdVzVKZmemn+qluHquqH5MZtewSfVJXNkRlFeMI+Lqv6dqTXnlYL3qs+of1V7KC9xSjVQ1V/2iWqn6O1GiML+prop7xnOUl6KruIKtfLqDTzeuKZGNeaotJr1PXD0DjAdeq9aZ9HtxHVKMZuLq6+bTTXz6p5oS3x+0PTxP6CfI0tT4QXhRszBKkvX1p3SKd6pD5N1QfSCvGLEHYe8PSoP2ES/GJdUT8jZKtmvrg8GqZ6ofjDdI3P1vGg/cV21XbVD9THm1gbrmk4eBxyoWBRdMI2+F90sBHcvX8OChIVwGwJvJJoEyO8gb5v1ygNUB97pLPj8juEDpLHQWVw9+LWE7S/G7uAzed2Z7H7OiroQXYJzxkMZ+wcC/w6YhLJHYOyxV3p9M/vcC+21b8upr8FZJuh6wR+K+LBaXwfvSVO9jqagLE8Vdv5l8eG/IA/Cx9xUirP1LyO/k/WXkl4uh4u6PQbVgGUVAc0r1XPUwmR3luMQHCctvzJc14jL6kj/J+zPIzwL2ocOqL+L2OAvqfEUegB9toGe0uPyF5Lfz/l/klwvc+182JR0voFyxaPOixPsAEwA+AssEc3wG3mzLn97nzi+FEMWeNF6hhyjUCYGe4spgpbB09P5a8msDUeLAjOrlr4lxTNwLmoWD4tqJI1kh9kt88LaJ8xHFJgh7Hqa/ZZb3U6NdIqjDNgj/P5p0AP4DNg04d6LMcvJ/9X4pK0SVanxG4awWY67EX8JCrBbXzunkWwrtebsk7ksLcRn1EW3iLeTlKwxeCIh4MAPwdrJJoEyhaLOcZ70hqkfk2WeKPWM40owh3zJcXJnM0SZAhj1cg9Pet4xV9SEvMEXijQ4eokWAzucyYVbhEB5oqVpg0gDLKkekSyVdX23t/Fa6qG6zKenBwxcWC85p3E6cETlQRBmOnN9KPE6oJjbLuJLQwVzOgjzM5ACCIHh4ESzwbOdek3QEGu7V23iYvXx/pO2Ll6WddSX25STosim3VZJRcbhuj/FArJ1nxEWpgfA83Y2X4oC4ZQ+/KMyBATgh6eXCgjMhZgf0Qlw9CHOZKnF551WPVU+T2dVMEPeNlQkzDV+EcODfncyuplg76wq2BB60IN6Lw8qFlxK/i5LZ1aBvYv2DmY2vXkfFXVvbUlsSmCWVQKW0s6zwNG+oVEo7ywaiHpyrGjqV0s6y0oONBkqltDMnJ6fi+Aq8IjBkAyhIBAAAAABJRU5ErkJggg==>

[image15]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAG8AAAAXCAYAAAAFtBHMAAAD9klEQVR4Xu2YWahNURjHP/MUZcx8JVOUJDLPXrwQGR6QN5IyFTKEDEXxYE7ipmQIxQNFPODBGJGUPJB4oAiZ5+9/1lrX2v+99j37XveenOxf/Ttn/de31157r7XX/tYWycjIyMjI+P9opDqv+qW6qaoRrU7FItUxVU9b7qE6pFpYFvGHraqvqleqoVSXj+Ni+vlA1ZDqCsEm1TfVd9VBqnPcUQ1T1VW1V61SvYlEJNNFdV3MNV6guhjtxAQ2sOXmtlyzLCIdG8Qc5wsXwbwVE+v4JOaG5KOOmDY72nItW25TFlH9oO997X93nyCf2p7vhImahhESba8PlWN8EPPE+NxSfSYvH2tU28XMRsw03FxmjMQ70yzghbiiekbeFkl3bFUwQPVC1djz+os5/23PAw9Vu1SbVW2prjzQ1lzyMPDXyCsDB0wlb4X1KwIGbBSbBDoSahfeDDYJxOwmb7D1CwFWB5zrPvnwuA+XqJyGVmLawa+Pe53FGC6mgt87s6yPpyItKyX/4KFNvC8Y+PfY9HBLJCaIT4n1J5FfXeB924S8qhq81RJvB5RK2M8lE6hw67hjivWxVKRluWqjmOPcCfdGIoz3jjwAH+++JNzav5j8ltZfRn6hGCTm/BhUHyyjSGjOqF6qHkerg5yS8CBh+Q35sk5MRW/yJ1p/OvnlgRt7jjy0sZ7Kr72yA36wg5bRYurnk9/U+vvILxQ49082JZ4vIC5ftnlZwvdgmxgfiWWE2bYCM9tnsvWRYPwNPChJF5F0ExzdxMTwtqOF9f0Jkg9kif1Sqrs9JsRJ1Q82Ezgqpp/YkiVxWMKDt1OMjyw2gnvn4fH3mWn92GiXQ2hviIvjwfvilR3wH7HpgbYRg6XZp4P1K7JClKjGpxT2aiHmSHgSJrFWTD+nke+T9M7bL2Ff6ompqIpsE/HYdLPHgxdqF94eNgnEJGWbhdzrDVQ9IS/fNbotzVjyfYaIiUmdbQJUYH/mc9b6PuNUvcjzQfySgOe3g5vP7bqnCptwR33VPK8MsKxyRrpU4u3l6+ff0Fp1l02JDx5/nMA+jfuJPSIniojhzPm9hPOEHKGnjBtxN5jjfD6Kyf4cI8XE83sDnn9zb0g8A3Xncp/aALYzfH6U/YmXpp+VJfTlxOmqF7dDolmxO67U80Con0j4kKU63PV08rwYR8S8n/CLYE4MwGmJLxcMlk3/ojpHq3OUiKm7qHqqeh6tzjFBzDdWxj1p+CKEDf+BaHWONP2sDMhoedCc+F3sVi5MSvwuiFbnwBYAYvBk46vXCTHHlrfUVgg8JcVAsfSzoGAmFAPF0s+CgawH+6p/nWLpZ0HpysY/SrH0MyMjo+j4DekZLXPVigR6AAAAAElFTkSuQmCC>

[image16]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGUAAAAXCAYAAAASloEFAAADM0lEQVR4Xu2YW6hNQRjHP/dEkkspeVHukpBbEimXSBFeJEVxXpRLHtzLpZQXKQ9KSSnkQUoKj/LiEkqUJCWEyCX36/c/M+uY9T/frL32aa+9Xdav/p2+/3xrz8yZWTOzRqSkpKSkpKT2dFNdUP1UXVW1SxfnYp3qpGq4j4eqjqnWtmRks0L1VPVNtZfK6gna/YxNz2vVLtUAVUfVRNX1VEY221RvVR9UK6ksRX9xg9HVx7193L4lIx+7xT0X6kYqI84p1ZMgxmC+CuKimal6Ib/b/Txd3AL3D8JkzMMd1cUgvq26HMQp3oub4SHXVJ/Iq8QO1QHVUdVWVYd0cSboHGYee7PJqweVBmWTuD6uprIseoh7loHXk02AgiXkbfZ+NWAgprOZg/1i1wXvPpt1IGtQsLS2hZsS7+NhNqf6ginkL/d+L/Kz2CJtG5QvEm+w5RdN1qB8ZSMnsb6YPjZhmGPIX+z9CeRngdd6j7jnjvi/h1IZNmbDJO4XTdagfFYdVL1RnRCXOzmVYRPri+nv9OYo8hd4fyn5WaxXnScPv4HTShZmwyTuFw3qxKZvgZPTrCAeLy6/0ooS64vpr/LmaPIXeX8G+dViVkrEcmJ+FuOqUAzU+ZLNDJCPwcoi1hfTT/aUSeQv8z6Oy3mxvm2+i1Epgc3TyjEbXIH5VSh2OkSdseN4JzYkXztjOabfxZu1OH0hn2eYWSmBs7uVA+8Hm3UA9eIjkUkm6kby8/Txndg58O6yCVCA74uQc94PmaMaQV5I3gavUXUO4n7SOgfA20DedoqLAPViI2dw44AyXj3g3SMPJ9EQTPpYH8eyCay3AvHCIMbSZP2DQ3B10DeIp4nLHxJ42Lus38EbgdNMwlxpnXPae5gwRYI6PrLp4TadNbzkRMtXNfCagnif96IcF7f+4y8SrfuqM6qHbBJYvpJ/OjQwXdzMFdU8NsV9r2DGXRL3bPd0cfMJJ7kKqTUjxe0jj1WPvHAPh2UnpI+4+vEmJYNn7aUPVMPIwzUWnkH/b4m7MbGerRr8YKOxlpb/miJmaTUMluq+nf55cLWP17eRNOI09kcziI0GUJM1uKSk5G/nF6MCAlSzc6lvAAAAAElFTkSuQmCC>

[image17]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAG8AAAAXCAYAAAAFtBHMAAAEDUlEQVR4Xu2ZWahOURTHFxHiwXAzZEyGpIQMyZQhURIyPIj7IpEhSRSlDA8eEQ9ClCmRMWNIKHNRSBkuHoSEzGRa/7v3vt/61lnnfN+93Xt1P+dXq++u/1pnn3322WefdfYlSklJSUlJ+f9oyHaG7Q/bDbZa2eFqYSrbB3J9mKFioBfbWbbebLXZOrKtYzsskxIYxvaAXPu7Vaw8jGT7Qa6dkyom2Uou5zlbOxVLohPbNXLH4noTaU0usYH3m3kfA1Rd3Gf7KHxMoMvCBxg09Eva66yMeBax/Rb+bHLHl5fR5G5KoDPZ7eBco4SPHBybi6GU3V5P5Uf4zLZPaTfZvimtqphMroNNlA6tufBxYcfJDd4qtkYilgu01c3Q1iotF9ZAHmDbLPzVFM0ba2gWyJmjNDzlV5VWBg6YorRlXq8O7pF9LminhD+IbaXw82U82e1/J1tPAvmNlXaIbYfwkYOVRAO9rRYFmKjIkRMWhNdZhCHkAhgYSbHXmyq9KsB5rM5pfSBV7OadJ7v9ErL1JEKf5ipNAt96V0HfoEXBCoq2BbaTrdNCcgEUAZKwlPVXelUQN4hhoAIDyBUa0PayfWG7KOJxvCe7/bgnPonBlOnXV/+rl29oR5UGoCcVOCi8rP5sIlsvfXcg0EPpE7w+TelVwXBy5yoSWkuvyU5jgj0TPkD8ktI0up3AHbL1XMynTJuwNSKGIg/aQaEFoD/UogAT0erPenI6CsssZvkAqhrJJK+PUHoSqFL75Gld/TGBR+SekECY1dbFSF5S7pwXZOfcJVtP4hjbOf83PlNCH+WnDXzr8wV60kTbQ3Z/NpLT6+hAeOdhSZJM93rkbifQnm1cnoblRzOP3EDv8j7O/ysTNrlALq+F0iVx77zHZOtxdKFoPj6vwg0M4O/Twg9A36JFQdw7bxvZOtUjF/iX1WYcOD9WBunrPuGTBlp9pUuWU/Q4UN5qE+/InVqkzA0M4O+4ahMbEXGgIENO3tUmQEBXQSe8LhnD1l1plQXOtVT4oZCSwN+vtLDTIUGxpZ9E5OjKGRqWQckSMpYnzxFyS62F7ANunO5TP0PrS9FCETkTlfaJ7a3SyrCeMt0Itsug6bzKAIOFdmcKDT6KJsltcrMz0IpcXrHQ4vqJd2OJ8ENBVFdoC7wmd3okoZ9tlI7lVz5RmCTIk6sB2rwlfGD1E8vtT+GH6+kgtAgovfF+wS+SMfM1mHlPtVhJYDsM533if7EVZhFmdXjirDwUC4u1yLxhe0WZVQV7oxrcYDl4mrCVCMPWHH71JANhKw9j9o7tSna4FHwCwDSogrHrhZ2buGusENe1UIDop6FgKNgL86CAS6oIayyoeoq0WGDk+jSpseDfH4WO3upKSUlJEfwFDUg05ChH4HcAAAAASUVORK5CYII=>